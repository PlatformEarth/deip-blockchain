#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <deip/chain/vote_object.hpp>
#include <deip/chain/dbs_vote.hpp>

#include "database_fixture.hpp"

namespace deip {
namespace chain {

class vote_service_fixture : public clean_database_fixture
{
public:
    vote_service_fixture()
            : data_service(db.obtain_service<dbs_vote>())
    {
    }

    void create_votes() {
        db.create<vote_object>([&](vote_object& r) {
            r.id = 1,
            r.voter = "alice";
            r.tokens_amount = 100;
            r.weight = 50 * DEIP_1_PERCENT;
            r.voting_power = 10 * DEIP_1_PERCENT;
            r.research_id = 1;
            r.research_content_id = 1;
            r.discipline_id = 1024;
        });

        db.create<vote_object>([&](vote_object& r) {
            r.id = 2,
            r.voter = "bob";
            r.tokens_amount = 1000;
            r.weight = 20 * DEIP_1_PERCENT;
            r.voting_power = 10 * DEIP_1_PERCENT;
            r.research_id = 2;
            r.research_content_id = 2;
            r.discipline_id = 1024;
        });
    }

    dbs_vote& data_service;
};

BOOST_FIXTURE_TEST_SUITE(vote_service, vote_service_fixture)

BOOST_AUTO_TEST_CASE(create_research_vote)
{
    try
    {
        auto time = db.head_block_time();
        auto& vote = data_service.create_vote(1024, "alice", 1, 1, 100, DEIP_100_PERCENT, 10 * DEIP_1_PERCENT, time);

        BOOST_CHECK(vote.discipline_id == 1024);
        BOOST_CHECK(vote.research_id == 1);
        BOOST_CHECK(vote.research_content_id == 1);
        BOOST_CHECK(vote.tokens_amount == 100);
        BOOST_CHECK(vote.weight == DEIP_100_PERCENT);
        BOOST_CHECK(vote.voting_power == 10 * DEIP_1_PERCENT);
        BOOST_CHECK(vote.voter == "alice");
        BOOST_CHECK(vote.voting_time == time);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_votes_by_discipline)
{
    try
    {
        create_votes();

        const auto& votes = data_service.get_votes_by_discipline(1024);


        BOOST_CHECK_EQUAL(votes.size(), 2);

        for (const chain::vote_object& vote : votes) {
            BOOST_CHECK(vote.discipline_id == 1024);
        }

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_votes_by_research)
{
    try
    {
        create_votes();

        const auto& votes = data_service.get_votes_by_research(1);


        BOOST_CHECK_EQUAL(votes.size(), 1);

        for (const chain::vote_object& vote : votes) {
            BOOST_CHECK(vote.research_id == 1);
        }

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_votes_by_research_content)
{
    try
    {
        create_votes();

        const auto& votes = data_service.get_votes_by_research_content(2);


        BOOST_CHECK_EQUAL(votes.size(), 1);

        for (const chain::vote_object& vote : votes) {
            BOOST_CHECK(vote.research_content_id == 2);
        }

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_votes_by_research_and_discipline)
{
    try
    {
        create_votes();

        const auto& votes = data_service.get_votes_by_research_and_discipline(1, 1024);


        BOOST_CHECK_EQUAL(votes.size(), 1);

        for (const chain::vote_object& vote : votes) {
            BOOST_CHECK(vote.research_id == 1);
            BOOST_CHECK(vote.discipline_id == 1024);
        }

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_votes_by_research_content_and_discipline)
{
    try
    {
        create_votes();

        const auto& votes = data_service.get_votes_by_research_content_and_discipline(1, 1024);


        BOOST_CHECK_EQUAL(votes.size(), 1);

        for (const chain::vote_object& vote : votes) {
            BOOST_CHECK(vote.research_content_id == 1);
            BOOST_CHECK(vote.discipline_id == 1024);
        }

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace chain
} // namespace deip

#endif