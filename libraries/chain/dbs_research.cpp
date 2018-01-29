#include <deip/chain/dbs_research.hpp>
#include <deip/chain/database.hpp>


namespace deip{
namespace chain{

dbs_research::dbs_research(database &db) : _base_type(db)
{
}

const research_object& dbs_research::create(const string &name, const string &abstract, const string &permlink,
                                            const research_group_id_type &research_group_id, const uint32_t &percent_for_review)
{
    const auto& new_research = db_impl().create<research_object>([&](research_object& r) {
        fc::from_string(r.name, name);
        fc::from_string(r.abstract, abstract);
        fc::from_string(r.permlink, permlink);
        r.research_group_id = research_group_id;
        r.percent_for_review = percent_for_review;
        r.is_finished = false;
        r.owned_tokens = DEIP_100_PERCENT;
        r.created = db_impl().head_block_time();
    });

    return new_research;
}

dbs_research::research_refs_type dbs_research::get_researches() const
{
    research_refs_type ret;

    auto idx = db_impl().get_index<research_index>().indicies();
    auto it = idx.cbegin();
    const auto it_end = idx.cend();
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

const research_object& dbs_research::get_research(const research_id_type& id) const
{
    return db_impl().get<research_object>(id);
}

const research_object& dbs_research::get_research_by_permlink(const string& permlink) const
{
    return db_impl().get<research_object, by_permlink>(permlink);
}

void dbs_research::check_research_existence(const research_id_type& id) const
{
    auto research = db_impl().find<research_object, by_id>(id);
    FC_ASSERT(research != nullptr, "Research with _id \"${1}\" must exist.", ("1", id));
}

void dbs_research::decrease_owned_tokens(const research_object& research, const share_type delta)
{
    FC_ASSERT((research.owned_tokens + delta > 0), "Cannot update research owned tokens (result amount < 0)");
    db_impl().modify(research, [&](research_object& r_o) { r_o.owned_tokens -= delta; });
}

}
}