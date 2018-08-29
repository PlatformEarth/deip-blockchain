#include <deip/chain/services/dbs_proposal.hpp>
#include <deip/chain/services/dbs_research_group.hpp>
#include <deip/chain/services/dbs_proposal_execution.hpp>
#include <deip/chain/database/database.hpp>

#include <tuple>

namespace deip {
namespace chain {

dbs_proposal::dbs_proposal(database& db)
    : _base_type(db)
{
}

const proposal_object& dbs_proposal::get_proposal(const proposal_id_type& id) const
{
    try {
        return db_impl().get<proposal_object>(id);
    }
    FC_CAPTURE_AND_RETHROW((id))
}

const dbs_proposal::proposal_ref_type
dbs_proposal::get_proposals_by_research_group_id(const research_group_id_type& research_group_id) const
{
    proposal_ref_type ret;

    auto it_pair = db_impl().get_index<proposal_index>().indicies().get<by_research_group_id>().equal_range(research_group_id);
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

const proposal_object& dbs_proposal::create_proposal(const dbs_proposal::action_t action,
                                                     const std::string json_data,
                                                     const account_name_type& creator,
                                                     const research_group_id_type& research_group_id,
                                                     const fc::time_point_sec expiration_time,
                                                     const share_type quorum_percent,
                                                     const size_t object_hash)
{
    const proposal_object& new_proposal = db_impl().create<proposal_object>([&](proposal_object& proposal) {
        proposal.action = action;
        fc::from_string(proposal.data, json_data);
        proposal.creator = creator;
        proposal.research_group_id = research_group_id;
        proposal.creation_time = db_impl().head_block_time();
        proposal.expiration_time = expiration_time;
        proposal.quorum_percent = quorum_percent;
        proposal.object_hash = object_hash;

        switch (action)
        {
            case invite_member :
                fc::json::from_string(json_data).as<invite_member_proposal_data_type>().validate();
                break;
            case dropout_member :
                fc::json::from_string(json_data).as<dropout_member_proposal_data_type>().validate();
                break;
            case change_research_review_share_percent :
                fc::json::from_string(json_data).as<change_research_review_share_percent_data_type>().validate();
                break;
            case change_quorum:
                fc::json::from_string(json_data).as<change_quorum_proposal_data_type>().validate();
                break;
            case start_research:
                fc::json::from_string(json_data).as<start_research_proposal_data_type>().validate();
                break;
            case send_funds:
                fc::json::from_string(json_data).as<send_funds_data_type>().validate();
                break;
            case rebalance_research_group_tokens:
                fc::json::from_string(json_data).as<rebalance_research_group_tokens_data_type>().validate();
                break;
            case create_research_material:
                fc::json::from_string(json_data).as<create_research_content_data_type>().validate();
                break;
            case start_research_token_sale:
                fc::json::from_string(json_data).as<start_research_token_sale_data_type>().validate();
                break;
        }
    });

    return new_proposal;
}

void dbs_proposal::remove(const proposal_object& proposal)
{
    db_impl().remove(proposal);
}

void dbs_proposal::check_proposal_existence(const proposal_id_type& proposal_id) const
{
    const auto& proposal = db_impl().get_index<proposal_index>().indices().get<by_id>();
    FC_ASSERT(proposal.find(proposal_id) != proposal.cend(), "Proposal \"${1}\" does not exist.", ("1", proposal_id));
}

bool dbs_proposal::is_expired(const proposal_object& proposal)
{
    return db_impl().head_block_time() > proposal.expiration_time && !proposal.is_completed;
}

void dbs_proposal::complete(const proposal_object &proposal) {
    db_impl().modify(proposal, [&](proposal_object& p) {
       p.is_completed = true;
    });
}

void dbs_proposal::clear_expired_proposals()
{
    const auto& proposal_expiration_index = db_impl().get_index<proposal_index>().indices().get<by_expiration_time>();

    while ((!proposal_expiration_index.empty())
           && (db_impl().head_block_time() > proposal_expiration_index.begin()->expiration_time)
           && (!proposal_expiration_index.begin()->is_completed))
    {
        db_impl().remove(*proposal_expiration_index.begin());
    }
}

const proposal_vote_object& dbs_proposal::vote_for(const proposal_id_type &proposal_id, const account_name_type &voter)
{
    auto& research_group_service = db_impl().obtain_service<dbs_research_group>();
    auto& proposal = get_proposal(proposal_id);

    auto& token = research_group_service.get_token_by_account_and_research_group(voter, proposal.research_group_id);
    auto wight = token.amount;

    db_impl().modify(proposal, [&](proposal_object& p) {
        p.voted_accounts.insert(voter);
    });

    return create_vote(voter, wight, proposal_id, proposal.research_group_id);
}

void dbs_proposal::remove_proposal_votes(const account_name_type& account,
                                                  const research_group_id_type& research_group_id)
{
    const auto& proposal_votes_idx
            = db_impl().get_index<proposal_vote_index>().indices().get<by_voter>();
    auto proposal_vote_itr = proposal_votes_idx.find(boost::make_tuple(account, research_group_id));

    while(proposal_vote_itr != proposal_votes_idx.end())
    {
        const auto& current_proposal_vote = *proposal_vote_itr;
        ++proposal_vote_itr;
        db_impl().remove(current_proposal_vote);
    }
}

const proposal_vote_object& dbs_proposal::create_vote(const account_name_type& voter,
                                                      const deip::chain::share_type weight,
                                                      const proposal_id_type& proposal_id,
                                                      const research_group_id_type& research_group_id)
{
    const proposal_vote_object& new_proposal_vote = db_impl().create<proposal_vote_object>([&](proposal_vote_object& proposal_vote) {
        proposal_vote.voter = voter;
        proposal_vote.weight = weight;
        proposal_vote.proposal_id = proposal_id;
        proposal_vote.research_group_id = research_group_id;
        proposal_vote.voting_time = db_impl().head_block_time();;
    });

    return new_proposal_vote;
}

const dbs_proposal::proposal_votes_ref_type  dbs_proposal::get_votes_for(const proposal_id_type &proposal_id) {
    proposal_votes_ref_type ret;

    auto it_pair = db_impl().get_index<proposal_vote_index>().indicies().get<by_proposal_id>().equal_range(proposal_id);
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

} // namespace chain
} // namespace deip
