#include <deip/account_by_key/account_by_key_plugin.hpp>
#include <deip/account_by_key/account_by_key_objects.hpp>
#include <deip/account_by_key/account_by_key_api.hpp>

#include <deip/chain/schema/account_object.hpp>
#include <deip/chain/database/database.hpp>
#include <deip/chain/operation_notification.hpp>

#include <graphene/schema/schema.hpp>
#include <graphene/schema/schema_impl.hpp>

namespace deip {
namespace account_by_key {

namespace detail {

class account_by_key_plugin_impl
{
public:
    account_by_key_plugin_impl(account_by_key_plugin& _plugin)
        : _self(_plugin)
    {
    }

    deip::chain::database& database()
    {
        return _self.database();
    }

    void pre_operation(const operation_notification& op_obj);
    void post_operation(const operation_notification& op_obj);
    void clear_cache();
    void cache_auths(const account_authority_object& a);
    void update_key_lookup(const account_authority_object& a);

    flat_set<public_key_type> cached_keys;
    account_by_key_plugin& _self;
};

struct pre_operation_visitor
{
    account_by_key_plugin& _plugin;

    pre_operation_visitor(account_by_key_plugin& plugin)
        : _plugin(plugin)
    {
    }

    typedef void result_type;

    template <typename T> void operator()(const T&) const
    {
    }

    void operator()(const create_account_operation& op) const
    {
        _plugin.my->clear_cache();
    }

    void operator()(const update_account_operation& op) const
    {
        _plugin.my->clear_cache();
        auto acct_itr = _plugin.database().find<account_authority_object, by_account>(op.account);
        if (acct_itr)
            _plugin.my->cache_auths(*acct_itr);
    }

    void operator()(const recover_account_operation& op) const
    {
        _plugin.my->clear_cache();
        auto acct_itr = _plugin.database().find<account_authority_object, by_account>(op.account_to_recover);
        if (acct_itr)
            _plugin.my->cache_auths(*acct_itr);
    }
};

struct pow2_work_get_account_visitor
{
    typedef const account_name_type* result_type;

    template <typename WorkType> result_type operator()(const WorkType& work) const
    {
        return &work.input.worker_account;
    }
};

struct post_operation_visitor
{
    account_by_key_plugin& _plugin;

    post_operation_visitor(account_by_key_plugin& plugin)
        : _plugin(plugin)
    {
    }

    typedef void result_type;

    template <typename T> void operator()(const T&) const
    {
    }

    void operator()(const create_account_operation& op) const
    {
        auto acct_itr = _plugin.database().find<account_authority_object, by_account>(op.new_account_name);
        if (acct_itr)
            _plugin.my->update_key_lookup(*acct_itr);
    }

    void operator()(const update_account_operation& op) const
    {
        auto acct_itr = _plugin.database().find<account_authority_object, by_account>(op.account);
        if (acct_itr)
            _plugin.my->update_key_lookup(*acct_itr);
    }

    void operator()(const recover_account_operation& op) const
    {
        auto acct_itr = _plugin.database().find<account_authority_object, by_account>(op.account_to_recover);
        if (acct_itr)
            _plugin.my->update_key_lookup(*acct_itr);
    }

    void operator()(const hardfork_operation& op) const
    {
        // removed due to hardfork removal
    }
};

void account_by_key_plugin_impl::clear_cache()
{
    cached_keys.clear();
}

void account_by_key_plugin_impl::cache_auths(const account_authority_object& a)
{
    for (const auto& item : a.owner.key_auths)
        cached_keys.insert(item.first);
    for (const auto& item : a.active.key_auths)
        cached_keys.insert(item.first);
}

void account_by_key_plugin_impl::update_key_lookup(const account_authority_object& a)
{
    auto& db = database();
    const auto& now = db.head_block_time();
    
    const auto& key_lookup_idx = db.get_index<key_lookup_index>()
      .indices()
      .get<by_key_deactivation_time>();

    flat_set<public_key_type> new_keys;

    // Construct the set of keys in the account's authority
    for (const auto& item : a.owner.key_auths)
        new_keys.insert(item.first);
    for (const auto& item : a.active.key_auths)
        new_keys.insert(item.first);

    // For each key that needs a lookup
    for (const auto& key : new_keys)
    {
        // If the key was not in the authority, add it to the lookup
        if (cached_keys.find(key) == cached_keys.end())
        {
            const auto& lookup_itr = key_lookup_idx.find(std::make_tuple(key, a.account, fc::time_point_sec::maximum()));
            if (lookup_itr == key_lookup_idx.end())
            {
                db.create<key_lookup_object>([&](key_lookup_object& o) {
                    o.key = key;
                    o.account = a.account;
                    o.deactivation_time = fc::time_point_sec::maximum();
                });
            }
        }
        else
        {
            // If the key was already in the auths, remove it from the set so we don't deactivate it
            cached_keys.erase(key);
        }
    }

    // Loop over the keys that were in authority but are no longer and remove them from the lookup
    for (const auto& key : cached_keys)
    {
        const auto& lookup_itr = key_lookup_idx.find(std::make_tuple(key, a.account, fc::time_point_sec::maximum()));
        if (lookup_itr != key_lookup_idx.end())
        {
            db.modify(*lookup_itr, [&](key_lookup_object& o) { o.deactivation_time = now; });
        }
    }

    cached_keys.clear();
}

void account_by_key_plugin_impl::pre_operation(const operation_notification& note)
{
    note.op.visit(pre_operation_visitor(_self));
}

void account_by_key_plugin_impl::post_operation(const operation_notification& note)
{
    note.op.visit(post_operation_visitor(_self));
}

} // detail

account_by_key_plugin::account_by_key_plugin(deip::app::application* app)
    : plugin(app)
    , my(new detail::account_by_key_plugin_impl(*this))
{
}

void account_by_key_plugin::plugin_set_program_options(boost::program_options::options_description& cli,
                                                       boost::program_options::options_description& cfg)
{
}

void account_by_key_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{
    try
    {
        ilog("Initializing account_by_key plugin");
        chain::database& db = database();

        db.pre_apply_operation.connect([&](const operation_notification& o) { my->pre_operation(o); });
        db.post_apply_operation.connect([&](const operation_notification& o) { my->post_operation(o); });

        db.add_plugin_index<key_lookup_index>();
    }
    FC_CAPTURE_AND_RETHROW()
}

void account_by_key_plugin::plugin_startup()
{
    app().register_api_factory<account_by_key_api>("account_by_key_api");
}
}
} // deip::account_by_key

DEIP_DEFINE_PLUGIN(account_by_key, deip::account_by_key::account_by_key_plugin)
