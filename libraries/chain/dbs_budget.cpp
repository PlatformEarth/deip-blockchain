#include <scorum/chain/dbs_budget.hpp>
#include <scorum/chain/database.hpp>
#include <scorum/chain/dbs_account.hpp>
#include <scorum/chain/account_object.hpp>

#include <scorum/chain/budget_objects.hpp>
#include <scorum/chain/database.hpp>

#include <tuple>

namespace scorum {
namespace chain {

dbs_budget::dbs_budget(database& db)
    : _base_type(db)
{
    db_impl().get_index<budget_index>().indicies().get<by_owner_name>();
}

dbs_budget::budget_refs_type dbs_budget::get_budgets() const
{
    budget_refs_type ret;

    auto idx = db_impl().get_index<budget_index>().indicies();
    auto it = idx.begin();
    auto it_end = idx.end();
    FC_ASSERT(it != it_end, "non any budget");
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        it++;
    }

    return ret;
}

dbs_budget::budget_refs_type dbs_budget::get_budgets(const account_name_type& owner) const
{
    budget_refs_type ret;

    auto it_pair = db_impl().get_index<budget_index>().indicies().get<by_owner_name>().equal_range(owner);
    auto it = it_pair.first;
    auto it_end = it_pair.second;
    FC_ASSERT(it != it_end, "budget not found");
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        it++;
    }

    return ret;
}

const budget_object& dbs_budget::get_any_budget(const account_name_type& owner) const
{
    auto it_pair = db_impl().get_index<budget_index>().indicies().get<by_owner_name>().equal_range(owner);
    auto it_begin = it_pair.first;
    auto it_end = it_pair.second;
    FC_ASSERT(it_begin != it_end, "budget not found");

    return *it_begin;
}

const budget_object& dbs_budget::create_fund_budget(const asset& balance_in_scorum,
                                                    const share_type& per_block,
                                                    const time_point_sec& deadline)
{
    FC_ASSERT(balance_in_scorum.symbol == SCORUM_SYMBOL, "invalid asset type (symbol)");
    FC_ASSERT(balance_in_scorum.amount > 0, "invalid balance_in_scorum");
    FC_ASSERT(per_block > 0, "invalid per_block");

    const dynamic_global_property_object& props = db_impl().get_dynamic_global_properties();

    auto head_block_num = db_impl().head_block_num();

    const budget_object& new_budget = db_impl().create<budget_object>([&](budget_object& budget) {
        budget.owner = SCORUM_ROOT_POST_PARENT;
        budget.created = props.time;
        budget.deadline = deadline;
        budget.balance = balance_in_scorum;
        budget.per_block = per_block;
        // allocate cash only after next block generation
        budget.last_allocated_block = head_block_num;
    });

    // there are not schedules for genesis budget
    return new_budget;
}

const budget_object& dbs_budget::create_budget(const account_object& owner,
                                               const optional<string>& content_permlink,
                                               const asset& balance_in_scorum,
                                               const share_type& per_block,
                                               const time_point_sec& deadline)
{
    FC_ASSERT(owner.name != SCORUM_ROOT_POST_PARENT, "not allowed for ordinary budget");
    FC_ASSERT(balance_in_scorum.symbol == SCORUM_SYMBOL, "invalid asset type (symbol)");
    FC_ASSERT(balance_in_scorum.amount > 0, "invalid balance_in_scorum");
    FC_ASSERT(per_block > 0, "invalid per_block");

    const dynamic_global_property_object& props = db_impl().get_dynamic_global_properties();

    dbs_account& account_service = db().obtain_service<dbs_account>();

    FC_ASSERT(owner.balance >= balance_in_scorum, "insufficient funds");

    account_service.decrease_balance(owner, balance_in_scorum);

    auto head_block_num = db_impl().head_block_num();

    const budget_object& new_budget = db_impl().create<budget_object>([&](budget_object& budget) {
        budget.owner = owner.name;
        if (content_permlink.valid())
        {
            fc::from_string(budget.content_permlink, *content_permlink);
        }
        budget.created = props.time;
        budget.deadline = deadline;
        budget.balance = balance_in_scorum;
        budget.per_block = per_block;
        // allocate cash only after next block generation
        budget.last_allocated_block = head_block_num;
    });

    return new_budget;
}

void dbs_budget::close_budget(const budget_object& budget)
{
    FC_ASSERT(budget.owner != SCORUM_ROOT_POST_PARENT, "not allowed for genesis budget");

    dbs_account& account_service = db().obtain_service<dbs_account>();

    const auto& owner = account_service.get_account(budget.owner);

    // withdraw all balance rest asset back to owner
    //
    asset repayable = budget.balance;
    if (repayable.amount > 0)
    {
        // check if input budget state != budget in DB
        repayable = _decrease_balance(budget, repayable);
        account_service.increase_balance(owner, repayable);
    }

    // delete budget
    //
    db_impl().remove(budget);
}

asset dbs_budget::allocate_cash(const budget_object& budget, const optional<time_point_sec>& now)
{
    asset ret(0, SCORUM_SYMBOL);

    dbs_budget::_time t = _get_now(now);
    auto head_block_num = db_impl().head_block_num();

    if (budget.last_allocated_block >= head_block_num)
    {
        return ret; // empty (allocation waits new block)
    }

    FC_ASSERT(budget.per_block > 0, "invalid per_block");
    ret = _decrease_balance(budget, asset(budget.per_block, SCORUM_SYMBOL));

    if (budget.deadline <= t)
    {
        close_budget(budget);
    }
    else
    {
        if (!_check_autoclose(budget))
        {
            db_impl().modify(budget, [&](budget_object& b) { b.last_allocated_block = head_block_num; });
        }
    }
    return ret;
}

asset dbs_budget::_decrease_balance(const budget_object& budget, const asset& balance_in_scorum)
{
    FC_ASSERT(balance_in_scorum.symbol == SCORUM_SYMBOL, "invalid asset type (symbol)");
    FC_ASSERT(balance_in_scorum.amount > 0, "invalid balance_in_scorum");

    asset ret(0, SCORUM_SYMBOL);

    db_impl().modify(budget, [&](budget_object& b) {
        if (b.balance.amount > 0 && balance_in_scorum.amount <= b.balance.amount)
        {
            b.balance -= balance_in_scorum;
            ret = balance_in_scorum;
        }
        else
        {
            ret = b.balance;
            b.balance.amount = 0;
        }
    });

    return ret;
}

bool dbs_budget::_check_autoclose(const budget_object& budget)
{
    if (budget.balance.amount <= 0)
    {
        close_budget(budget);
        return true;
    }
    else
    {
        return false;
    }
}
}
}
