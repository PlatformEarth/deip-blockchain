#include <deip/chain/dbs_research_group_invite.hpp>
#include <deip/chain/database.hpp>


namespace deip{
namespace chain{

dbs_research_group_invite::dbs_research_group_invite(database &db) : _base_type(db)
{
}

const research_group_invite_object& dbs_research_group_invite::create(const account_name_type& account_name,
                                                                      const research_group_id_type& research_group_id,
                                                                      const share_type research_group_token_amount)
{
    const auto& new_research_group_invite = db_impl().create<research_group_invite_object>([&](research_group_invite_object& rgi_o) {
        rgi_o.account_name = account_name;
        rgi_o.research_group_id = research_group_id;
        rgi_o.research_group_token_amount = research_group_token_amount;
    });

    return new_research_group_invite;
}

const research_group_invite_object& dbs_research_group_invite::get(const research_group_invite_id_type& research_group_invite_id)
{
    return db_impl().get<research_group_invite_object>(research_group_invite_id);
}

const research_group_invite_object&
    dbs_research_group_invite::get_research_group_invite_by_account_name_and_research_group_id(const account_name_type& account_name,
                                                                                               const research_group_id_type& research_group_id)
{
    return db_impl().get<research_group_invite_object, by_account_and_research_group_id>(boost::make_tuple(account_name, research_group_id));
}

void dbs_research_group_invite::check_research_group_invite_existence(const research_group_invite_id_type& research_group_invite_id)
{
    auto research_group_invite = db_impl().find<research_group_invite_object, by_id>(research_group_invite_id);
    FC_ASSERT(research_group_invite != nullptr, "Research group invite with id \"${1}\" must exist.", ("1", research_group_invite_id));
}

dbs_research_group_invite::research_group_invite_refs_type
    dbs_research_group_invite::get_research_group_invites_by_account_name(const account_name_type& account_name)
{
    research_group_invite_refs_type ret;

    auto it_pair = db_impl().get_index<research_group_invite_index>().indicies().get<by_account_name>().equal_range(account_name);
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

}
}