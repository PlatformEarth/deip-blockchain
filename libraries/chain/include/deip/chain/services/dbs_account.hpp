#pragma once

#include "dbs_base_impl.hpp"

#include <deip/chain/schema/account_object.hpp>

namespace deip {
namespace chain {

// DB operations with account_*** objects
//
class dbs_account : public dbs_base
{
    friend class dbservice_dbs_factory;

protected:
    explicit dbs_account(database& db);

public:
    using account_optional_ref_type = fc::optional<std::reference_wrapper<const account_object>>;
    using accounts_refs_type = std::vector<std::reference_wrapper<const account_object>>;

    const account_object& get_account(const account_name_type&) const;
    
    const account_optional_ref_type get_account_if_exists(const account_name_type&) const;

    const account_authority_object& get_account_authority(const account_name_type&) const;

    void check_account_existence(const account_name_type&,
                                 const optional<const char*>& context_type_name = optional<const char*>()) const;

    void check_account_existence(const authority::account_authority_map&,
                                 const optional<const char*>& context_type_name = optional<const char*>()) const;

    const bool account_exists(const account_name_type& name) const;

    const account_object& create_account_by_faucets(const account_name_type& new_account_name,
                                                    const account_name_type& creator_name,
                                                    const public_key_type& memo_key,
                                                    const fc::optional<string>& json_metadata,
                                                    const authority& owner,
                                                    const authority& active,
                                                    const flat_map<uint16_t, authority>& active_overrides,
                                                    const asset& fee_in_deips,
                                                    const flat_set<deip::protocol::account_trait>& traits,
                                                    const bool& is_user_account = true);

    void update_acount(const account_object& account,
                       const public_key_type& memo_key,
                       const string& json_metadata,
                       const optional<flat_set<deip::protocol::account_trait>>& traits,
                       const optional<time_point_sec>& now = optional<time_point_sec>());

    void update_withdraw(const account_object& account,
                         const share_type& common_tokens_withdraw_rate,
                         const time_point_sec& next_common_tokens_withdrawal,
                         const share_type& to_withdrawn,
                         const optional<share_type>& withdrawn = optional<share_type>());

    void increase_withdraw_routes(const account_object& account);
    void decrease_withdraw_routes(const account_object& account);

    void increase_witnesses_voted_for(const account_object& account);
    void decrease_witnesses_voted_for(const account_object& account);

    void update_owner_authority(const account_object& account,
                                const authority& owner_authority,
                                const optional<time_point_sec>& now = optional<time_point_sec>());

    void update_active_authority(const account_object& account,
                                 const authority& active_authority);

    void update_active_overrides_authorities(const account_object& account,
                                             const flat_map<uint16_t, optional<authority>>& auth_overrides);

    void add_to_active_authority(const account_object& account,
                                 const account_name_type& member,
                                 const weight_type& weight = DEIP_MIN_AUTH_THRESHOLD);

    void remove_from_active_authority(const account_object& account,
                                      const account_name_type& member);

    void create_account_recovery(const account_name_type& account_to_recover_name,
                                 const authority& new_owner_authority,
                                 const optional<time_point_sec>& now = optional<time_point_sec>());

    void submit_account_recovery(const account_object& account_to_recover,
                                 const authority& new_owner_authority,
                                 const authority& recent_owner_authority,
                                 const optional<time_point_sec>& now = optional<time_point_sec>());

    void change_recovery_account(const account_object& account_to_recover,
                                 const account_name_type& new_recovery_account,
                                 const optional<time_point_sec>& now = optional<time_point_sec>());

    void update_voting_proxy(const account_object& account, const optional<account_object>& proxy_account);

    /** clears all vote records for a particular account but does not update the
    * witness vote totals.  Vote totals should be updated first via a call to
    * adjust_proxied_witness_votes( a, -a.witness_vote_weight() )
    */
    void clear_witness_votes(const account_object& account);

    /** this updates the votes for witnesses as a result of account voting proxy changing */
    void adjust_proxied_witness_votes(const account_object& account,
                                      const std::array<share_type, DEIP_MAX_PROXY_RECURSION_DEPTH + 1>& delta,
                                      int depth = 0);

    /** this updates the votes for all witnesses as a result of account VESTS changing */
    void adjust_proxied_witness_votes(const account_object& account, share_type delta, int depth = 0);

    void increase_common_tokens(const account_object &account, const share_type &amount);
    void decrease_common_tokens(const account_object &account, const share_type &amount);

    void adjust_expertise_tokens_throughput(const account_object& account, const share_type& delta);

    accounts_refs_type get_accounts_by_expert_discipline(const discipline_id_type& discipline_id) const;

    void process_account_recovery();

    const accounts_refs_type lookup_accounts(const string& lower_bound_name, uint32_t limit) const;

    const accounts_refs_type lookup_user_accounts(const string& lower_bound_name, uint32_t limit) const;

    const accounts_refs_type lookup_research_group_accounts(const string& lower_bound_name, uint32_t limit) const;

private:
    const account_object& get_account(const account_id_type &) const;
};
} // namespace chain
} // namespace deip
