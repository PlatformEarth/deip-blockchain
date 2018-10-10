#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <deip/chain/schema/expertise_allocation_proposal_object.hpp>
#include <deip/chain/services/dbs_expertise_allocation_proposal.hpp>

#include "database_fixture.hpp"

namespace deip {
namespace chain {

class expertise_allocation_proposal_service_fixture : public clean_database_fixture
{
public:
    expertise_allocation_proposal_service_fixture()
            : data_service(db.obtain_service<dbs_expertise_allocation_proposal>())
    {
    }

    void expertise_allocation_proposals()
    {
        db.create<expertise_allocation_proposal_object>([&](expertise_allocation_proposal_object& eap_o) {
            eap_o.id = 0;
            eap_o.initiator = "alice";
            eap_o.claimer = "bob";
            eap_o.discipline_id = 1;
            eap_o.total_voted_expertise = 0;
            eap_o.description = "test1";
        });
        db.create<expertise_allocation_proposal_object>([&](expertise_allocation_proposal_object& eap_o) {
            eap_o.id = 1;
            eap_o.initiator = "alice";
            eap_o.claimer = "bob";
            eap_o.discipline_id = 2;
            eap_o.total_voted_expertise = 0;
            eap_o.description = "test2";
        });
        db.create<expertise_allocation_proposal_object>([&](expertise_allocation_proposal_object& eap_o) {
            eap_o.id = 2;
            eap_o.initiator = "jack";
            eap_o.claimer = "bob";
            eap_o.discipline_id = 2;
            eap_o.total_voted_expertise = 0;
            eap_o.description = "test3";
        });
    }

    void expertise_allocation_proposal_votes()
    {
        db.create<expertise_allocation_proposal_vote_object>([&](expertise_allocation_proposal_vote_object& eapv_o) {
            eapv_o.id = 0;
            eapv_o.expertise_allocation_proposal_id = 0;
            eapv_o.discipline_id = 1;
            eapv_o.voter = "john";
            eapv_o.weight = DEIP_1_PERCENT * 50;
        });
        db.create<expertise_allocation_proposal_vote_object>([&](expertise_allocation_proposal_vote_object& eapv_o) {
            eapv_o.id = 1;
            eapv_o.expertise_allocation_proposal_id = 1;
            eapv_o.discipline_id = 2;
            eapv_o.voter = "mike";
            eapv_o.weight = DEIP_1_PERCENT * 50;
        });
        db.create<expertise_allocation_proposal_vote_object>([&](expertise_allocation_proposal_vote_object& eapv_o) {
            eapv_o.id = 2;
            eapv_o.expertise_allocation_proposal_id = 2;
            eapv_o.discipline_id = 2;
            eapv_o.voter = "cyntia";
            eapv_o.weight = DEIP_1_PERCENT * 50;
        });
    }
    dbs_expertise_allocation_proposal& data_service;
};

BOOST_FIXTURE_TEST_SUITE(expertise_allocation_proposal_service, expertise_allocation_proposal_service_fixture)

BOOST_AUTO_TEST_CASE(create)
{
    ACTORS_WITH_EXPERT_TOKENS((alice)(bob))
    try
    {
        auto expertise_allocation_proposal = data_service.create("alice", "bob", 1, 100, "test");

        BOOST_CHECK(expertise_allocation_proposal.id == 0);
        BOOST_CHECK(expertise_allocation_proposal.initiator == "alice");
        BOOST_CHECK(expertise_allocation_proposal.claimer == "bob");
        BOOST_CHECK(expertise_allocation_proposal.creation_time == db.head_block_time());
        BOOST_CHECK(expertise_allocation_proposal.expiration_time == db.head_block_time() + DAYS_TO_SECONDS(14));
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get)
{
    ACTORS((alice)(bob)(john))

    try
    {
        expertise_allocation_proposals();
        auto expertise_allocation_proposal = data_service.get(0);

        BOOST_CHECK(expertise_allocation_proposal.initiator == "alice");
        BOOST_CHECK(expertise_allocation_proposal.claimer == "bob");
        BOOST_CHECK(expertise_allocation_proposal.discipline_id == 1);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_by_initiator)
{
    ACTORS((alice)(bob)(john))

    try
    {
        expertise_allocation_proposals();
        auto expertise_allocation_proposals = data_service.get_by_initiator("alice");

        BOOST_CHECK(expertise_allocation_proposals.size() == 2);
        BOOST_CHECK(std::any_of(expertise_allocation_proposals.begin(), expertise_allocation_proposals.end(),
                                [](std::reference_wrapper<const expertise_allocation_proposal_object> wrapper){
            const expertise_allocation_proposal_object &eap_o = wrapper.get();
            return eap_o.initiator == "alice" && eap_o.claimer == "bob" && eap_o.discipline_id == 1;
        }));
        BOOST_CHECK(std::any_of(expertise_allocation_proposals.begin(), expertise_allocation_proposals.end(),
                                [](std::reference_wrapper<const expertise_allocation_proposal_object> wrapper){
                                    const expertise_allocation_proposal_object &eap_o = wrapper.get();
                                    return eap_o.initiator == "alice" && eap_o.claimer == "bob" && eap_o.discipline_id == 2;
                                }));
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_by_discipline_and_claimer)
{
    ACTORS((alice)(bob)(john))

    try
    {
        expertise_allocation_proposals();
        auto expertise_allocation_proposals = data_service.get_by_discipline_and_claimer(2, "bob");

        BOOST_CHECK(expertise_allocation_proposals.size() == 2);
        BOOST_CHECK(std::any_of(expertise_allocation_proposals.begin(), expertise_allocation_proposals.end(),
                                [](std::reference_wrapper<const expertise_allocation_proposal_object> wrapper){
            const expertise_allocation_proposal_object &eap_o = wrapper.get();
            return eap_o.initiator == "alice" && eap_o.claimer == "bob" && eap_o.discipline_id == 2;
        }));
        BOOST_CHECK(std::any_of(expertise_allocation_proposals.begin(), expertise_allocation_proposals.end(),
                                [](std::reference_wrapper<const expertise_allocation_proposal_object> wrapper){
                                    const expertise_allocation_proposal_object &eap_o = wrapper.get();
                                    return eap_o.initiator == "jack" && eap_o.claimer == "bob" && eap_o.discipline_id == 2;
                                }));
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_by_discipline_initiator_and_claimer)
{
    ACTORS((alice)(bob)(john))

    try
    {
        expertise_allocation_proposals();
        auto expertise_allocation_proposal = data_service.get_by_discipline_initiator_and_claimer(1, "alice", "bob");

        BOOST_CHECK(expertise_allocation_proposal.initiator == "alice");
        BOOST_CHECK(expertise_allocation_proposal.claimer == "bob");
        BOOST_CHECK(expertise_allocation_proposal.discipline_id == 1);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_by_discipline_id)
{
    ACTORS((alice)(bob)(john))

    try
    {
        expertise_allocation_proposals();
        auto expertise_allocation_proposals = data_service.get_by_discipline_id(2);

        BOOST_CHECK(expertise_allocation_proposals.size() == 2);
        BOOST_CHECK(std::any_of(expertise_allocation_proposals.begin(), expertise_allocation_proposals.end(),
                                [](std::reference_wrapper<const expertise_allocation_proposal_object> wrapper){
                                    const expertise_allocation_proposal_object &eap_o = wrapper.get();
                                    return eap_o.initiator == "alice" && eap_o.claimer == "bob" && eap_o.discipline_id == 2;
                                }));
        BOOST_CHECK(std::any_of(expertise_allocation_proposals.begin(), expertise_allocation_proposals.end(),
                                [](std::reference_wrapper<const expertise_allocation_proposal_object> wrapper){
                                    const expertise_allocation_proposal_object &eap_o = wrapper.get();
                                    return eap_o.initiator == "jack" && eap_o.claimer == "bob" && eap_o.discipline_id == 2;
                                }));
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(check_existence_by_discipline_initiator_and_claimer)
{
    expertise_allocation_proposals();

    BOOST_CHECK_NO_THROW(data_service.check_existence_by_discipline_initiator_and_claimer(1, "alice", "bob"));
    BOOST_CHECK_THROW(data_service.check_existence_by_discipline_initiator_and_claimer(3, "jack", "alice"), fc::assert_exception);
}

BOOST_AUTO_TEST_CASE(is_exists_y_discipline_initiator_and_claimer)
{
    expertise_allocation_proposals();

    BOOST_CHECK(data_service.is_exists_by_discipline_initiator_and_claimer(1, "alice", "bob") == true);
    BOOST_CHECK(data_service.is_exists_by_discipline_initiator_and_claimer(3, "jack", "alice") == false);
}

BOOST_AUTO_TEST_CASE(increase_total_voted_expertise)
{
    ACTORS((alice)(bob)(john)(mike))

    try
    {
        expertise_allocation_proposals();
        auto& expertise_allocation_proposal = data_service.get_by_discipline_initiator_and_claimer(2, "alice", "bob");

        data_service.downvote(expertise_allocation_proposal, "john", 1000);
        BOOST_CHECK(expertise_allocation_proposal.total_voted_expertise == -1000);

        data_service.upvote(expertise_allocation_proposal, "john", 1000);
        BOOST_CHECK(expertise_allocation_proposal.total_voted_expertise == 1000);

        data_service.upvote(expertise_allocation_proposal, "mike", 1000);
        BOOST_CHECK(expertise_allocation_proposal.total_voted_expertise == 2000);

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(decrease_total_voted_expertise)
{
    ACTORS((alice)(bob)(john)(mike))

    try {
        expertise_allocation_proposals();
        auto &expertise_allocation_proposal = data_service.get_by_discipline_initiator_and_claimer(2, "alice", "bob");

        data_service.upvote(expertise_allocation_proposal, "john", 1000);
        BOOST_CHECK(expertise_allocation_proposal.total_voted_expertise == 1000);

        data_service.downvote(expertise_allocation_proposal, "john", 1000);
        BOOST_CHECK(expertise_allocation_proposal.total_voted_expertise == -1000);

        data_service.downvote(expertise_allocation_proposal, "mike", 1000);
        BOOST_CHECK(expertise_allocation_proposal.total_voted_expertise == -2000);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(delete_by_discipline_and_claimer)
{
    ACTORS((alice)(bob)(john)(mike))

    try {
        expertise_allocation_proposals();

        data_service.delete_by_discipline_and_claimer(2, "bob");

        BOOST_CHECK(db.get_index<expertise_allocation_proposal_index>().indices().size() == 1);
    }
    FC_LOG_AND_RETHROW()
}


// Expertise allocation proposal votes

BOOST_AUTO_TEST_CASE(create_vote)
{
    ACTORS_WITH_EXPERT_TOKENS((alice)(bob))
    try
    {
        auto expertise_allocation_proposal_vote = data_service.create_vote(0, 1, "alice", 5000);

        BOOST_CHECK(expertise_allocation_proposal_vote.id == 0);
        BOOST_CHECK(expertise_allocation_proposal_vote.expertise_allocation_proposal_id == 0);
        BOOST_CHECK(expertise_allocation_proposal_vote.discipline_id == 1);
        BOOST_CHECK(expertise_allocation_proposal_vote.voter == "alice");
        BOOST_CHECK(expertise_allocation_proposal_vote.weight == 5000);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace chain
} // namespace deip

#endif
