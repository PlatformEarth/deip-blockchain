#include <deip/chain/dbs_vote.hpp>
#include <deip/chain/database.hpp>

#include <tuple>

namespace deip {
namespace chain {

dbs_vote::dbs_vote(database &db)
        : _base_type(db)
{
}

dbs_vote::vote_refs_type dbs_vote::get_votes_by_discipline(const discipline_id_type& discipline_id) const
{
    vote_refs_type ret;

    auto it_pair = db_impl().get_index<vote_index>().indicies().get<by_discipline_id>().equal_range(discipline_id);
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

dbs_vote::vote_refs_type dbs_vote::get_votes_by_type_and_target(const vote_target_type& vote_type, const int64_t& vote_for_id) const
{
    vote_refs_type ret;

    auto it_pair = db_impl().get_index<vote_index>().indicies().get<by_type_and_target_id>().equal_range(std::make_tuple(vote_type, vote_for_id));
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

const vote_object& dbs_vote::create_vote(const discipline_id_type &discipline_id, const account_name_type &voter,
                                         const vote_target_type &vote_type, const int64_t &vote_for_id,
                                         const share_type &weight, const time_point_sec &voting_time)
{
    FC_ASSERT(weight != 0, "Cannot vote with 0 weight");
    FC_ASSERT(voting_time <= db_impl().head_block_time(), "Voting time cannot be in future");

    const auto& new_vote = db_impl().create<vote_object>([&](vote_object& v) {
        v.discipline_id = discipline_id;
        v.vote_type = vote_type,
        v.vote_for_id = vote_for_id,
        v.voter = voter;
        v.weight = weight;
        v.voting_time = voting_time;
    });

    return new_vote;
}

} //namespace chain
} //namespace deip