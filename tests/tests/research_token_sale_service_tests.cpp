#ifdef IS_TEST_NET
#include <boost/test/unit_test.hpp>

#include <deip/chain/research_token_sale_object.hpp>
#include <deip/chain/dbs_research_token_sale.hpp>

#include "database_fixture.hpp"

#define RESEARCH_ID 1
#define START_TIME fc::time_point_sec(1581174151)
#define END_TIME fc::time_point_sec(1583679751)
#define BALANCE_TOKENS 100
#define SOFT_CAP 20
#define HARD_CAP 90

namespace deip {
namespace chain {

class research_token_sale_fixture : public clean_database_fixture
{
public:
    research_token_sale_fixture()
            : data_service(db.obtain_service<dbs_research_token_sale>())
    {
    }

    void create_research_token_sale() {
        db.create<research_token_sale_object>([&](research_token_sale_object& ts) {
            ts.research_id = RESEARCH_ID;
            ts.start_time = START_TIME;
            ts.end_time = END_TIME;
            ts.balance_tokens = BALANCE_TOKENS;
            ts.soft_cap = SOFT_CAP;
            ts.hard_cap = HARD_CAP;
        });
    }

    void create_research_token_sale_contributions()
    {
        db.create<research_token_sale_contribution_object>([&](research_token_sale_contribution_object& d) {
            d.id = 1;
            d.owner = "alice";
            d.amount = 100;
            d.research_token_sale_id = 1;
        });

        db.create<research_token_sale_contribution_object>([&](research_token_sale_contribution_object& d) {
            d.id = 2;
            d.owner = "bob";
            d.amount = 200;
            d.research_token_sale_id = 1;
        });

        db.create<research_token_sale_contribution_object>([&](research_token_sale_contribution_object& d) {
            d.id = 3;
            d.owner = "bob";
            d.amount = 300;
            d.research_token_sale_id = 2;
        });
    }

    dbs_research_token_sale& data_service;
};

BOOST_FIXTURE_TEST_SUITE(research_token_sale_service_tests, research_token_sale_fixture)

BOOST_AUTO_TEST_CASE(create_research_token_sale)
{
    try
    {
        auto& research_token_sale = data_service.start_research_token_sale(RESEARCH_ID, START_TIME, END_TIME, BALANCE_TOKENS, SOFT_CAP, HARD_CAP);

        BOOST_CHECK(research_token_sale.research_id == RESEARCH_ID);
        BOOST_CHECK(research_token_sale.start_time == START_TIME);
        BOOST_CHECK(research_token_sale.end_time == END_TIME);
        BOOST_CHECK(research_token_sale.total_amount == 0);
        BOOST_CHECK(research_token_sale.balance_tokens == BALANCE_TOKENS);
        BOOST_CHECK(research_token_sale.soft_cap == SOFT_CAP);
        BOOST_CHECK(research_token_sale.hard_cap == HARD_CAP);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_research_token_sale_by_id)
{
    try
    {
        create_research_token_sale();

        auto& research_token_sale = data_service.get_research_token_sale_by_id(0);

        BOOST_CHECK(research_token_sale.research_id == RESEARCH_ID);
        BOOST_CHECK(research_token_sale.start_time == START_TIME);
        BOOST_CHECK(research_token_sale.end_time == END_TIME);
        BOOST_CHECK(research_token_sale.total_amount == 0);
        BOOST_CHECK(research_token_sale.balance_tokens == BALANCE_TOKENS);
        BOOST_CHECK(research_token_sale.soft_cap == SOFT_CAP);
        BOOST_CHECK(research_token_sale.hard_cap == HARD_CAP);

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_research_token_sale_by_research_id)
{
    try
    {
        create_research_token_sale();

        auto& research_token_sale = data_service.get_research_token_sale_by_research_id(RESEARCH_ID);

        BOOST_CHECK(research_token_sale.research_id == RESEARCH_ID);
        BOOST_CHECK(research_token_sale.start_time == START_TIME);
        BOOST_CHECK(research_token_sale.end_time == END_TIME);
        BOOST_CHECK(research_token_sale.total_amount == 0);
        BOOST_CHECK(research_token_sale.balance_tokens == BALANCE_TOKENS);
        BOOST_CHECK(research_token_sale.soft_cap == SOFT_CAP);
        BOOST_CHECK(research_token_sale.hard_cap == HARD_CAP);

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_research_token_sale_by_end_time)
{
    try
    {
        create_research_token_sale();
        data_service.start_research_token_sale(2, fc::time_point_sec(1581177654), END_TIME,
                                                                           90, 60, 90);

        auto research_token_sales = data_service.get_research_token_sale_by_end_time(END_TIME);

        BOOST_CHECK(research_token_sales.size() == 2);

        BOOST_CHECK(std::any_of(research_token_sales.begin(), research_token_sales.end(),
                                [](std::reference_wrapper<const research_token_sale_object> wrapper) {
                                    const research_token_sale_object& research_token_sale = wrapper.get();
                                    return research_token_sale.id == 0 && research_token_sale.research_id == 1
                                        && research_token_sale.start_time == START_TIME
                                        && research_token_sale.end_time == END_TIME
                                        && research_token_sale.total_amount == 0
                                        && research_token_sale.balance_tokens == BALANCE_TOKENS
                                        && research_token_sale.soft_cap == SOFT_CAP
                                        && research_token_sale.hard_cap == HARD_CAP;
                                }));

        BOOST_CHECK(std::any_of(research_token_sales.begin(), research_token_sales.end(),
                                [](std::reference_wrapper<const research_token_sale_object> wrapper) {
                                    const research_token_sale_object& research_token_sale = wrapper.get();
                                    return research_token_sale.id == 1 && research_token_sale.research_id == 2
                                        && research_token_sale.start_time == fc::time_point_sec(1581177654)
                                        && research_token_sale.end_time == END_TIME
                                        && research_token_sale.total_amount == 0
                                        && research_token_sale.balance_tokens == 90
                                        && research_token_sale.soft_cap == 60
                                        && research_token_sale.hard_cap == 90;
                                }));
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(create_research_token_sale_contribution)
{
    try
    {
        auto& research_token_sale_contribution =
                data_service.create_research_token_sale_contribution(1, "alice", fc::time_point_sec(0xffffffff), 100);

        BOOST_CHECK(research_token_sale_contribution.research_token_sale_id == 1);
        BOOST_CHECK(research_token_sale_contribution.owner == "alice");
        BOOST_CHECK(research_token_sale_contribution.contribution_time == fc::time_point_sec(0xffffffff));
        BOOST_CHECK(research_token_sale_contribution.amount == 100);

    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_research_token_sale_contribution_by_id_test)
{
    try
    {
        create_research_token_sale_contributions();

        auto& research_token_sale_contribution = data_service.get_research_token_sale_contribution_by_id(1);

        BOOST_CHECK(research_token_sale_contribution.owner == "alice");
        BOOST_CHECK(research_token_sale_contribution.amount == 100);
        BOOST_CHECK(research_token_sale_contribution.research_token_sale_id == 1);

        BOOST_CHECK_THROW(data_service.get_research_token_sale_contribution_by_id(4), boost::exception);
        BOOST_CHECK_THROW(data_service.get_research_token_sale_contribution_by_id(0), boost::exception);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_research_token_sale_contribution_by_research_token_sale_id_test)
{
    try
    {
        create_research_token_sale_contributions();

        auto research_token_sale_contribution = data_service.get_research_token_sale_contributions_by_research_token_sale_id(1);

        BOOST_CHECK(research_token_sale_contribution.size() == 2);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(get_research_token_sale_contribution_by_account_name_test)
{
    try
    {
        create_research_token_sale_contributions();

        auto research_token_sale_contribution = data_service.get_research_token_sale_contributions_by_account_name("bob");

        BOOST_CHECK(research_token_sale_contribution.size() == 2);
    }
    FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace chain
} // namespace deip

#endif
