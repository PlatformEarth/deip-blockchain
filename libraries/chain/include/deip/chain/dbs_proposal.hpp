#pragma once

#include <deip/chain/dbs_base_impl.hpp>

#include <vector>
#include <set>
#include <functional>

#include <deip/chain/proposal_object.hpp>
#include <deip/chain/account_object.hpp>
#include <deip/chain/proposal_vote_object.hpp>

namespace deip {
namespace chain {

class proposal_object;

class dbs_proposal : public dbs_base
{
    friend class dbservice_dbs_factory;

    dbs_proposal() = delete;
protected:
    explicit dbs_proposal(database& db);

public:

    typedef deip::protocol::proposal_action_type action_t;
    typedef deip::protocol::account_name_type account_t;

    /** Get proposal by id
     */
    const proposal_object& get_proposal(proposal_id_type id) const;

    /** Create proposal object.
     *
     * @param action - type of proposal
     * @param json_data - data attached to particular action type
     * @param initiator - person who promote this proposal
     * @param lifetime - lifetime of proposal !!!(will be changed to end date)
     * @returns proposal object
     */
    const proposal_object& create_proposal(const action_t action,
                                           const std::string json_data,
                                           const account_t initiator,
                                           const research_group_id_type research_group_id,
                                           const fc::time_point_sec expiration_time,
                                           const u_int16_t quorum_percent);

    void remove(const proposal_object& proposal);

    void check_proposal_existence(proposal_id_type proposal_id) const;

    bool is_expired(const proposal_object& proposal);

    void clear_expired_proposals();

    /* Create proposal vote object
     * @param voter - person who vote
     * @param weight - weight of persons vote
     * @param id - id of proposal
     * */

    const proposal_vote_object& create_vote(const account_t voter,
                                            const deip::chain::share_type weight,
                                            const proposal_id_type id,
                                            const research_group_id_type research_group_id);

    void remove_proposal_votes(const account_name_type account_t,
                                                  const research_group_id_type research_group_id);

};

} // namespace chain
} // namespace deip