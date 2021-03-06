#include <deip/chain/services/dbs_discipline.hpp>
#include <deip/chain/services/dbs_research_discipline_relation.hpp>
#include <deip/chain/database/database.hpp>

namespace deip {
namespace chain {

dbs_research_discipline_relation::dbs_research_discipline_relation(database &db)
        : _base_type(db)
{
}

const research_discipline_relation_object& dbs_research_discipline_relation::create_research_relation(const research_id_type& research_id, const discipline_id_type& discipline_id)
{
    const auto& discipline_service = db_impl().obtain_service<dbs_discipline>();

    FC_ASSERT(discipline_id != 0, "Research cannot be in root discipline");

    const auto& discipline = discipline_service.get_discipline(discipline_id);
    const auto& new_relation = db_impl().create<research_discipline_relation_object>([&](research_discipline_relation_object& r) {
        r.research_id = research_id;
        r.discipline_id = discipline_id;
        r.discipline_external_id = discipline.external_id;
    });

    return new_relation;
}

const research_discipline_relation_object& dbs_research_discipline_relation::update_votes_count(const research_id_type& research_id, const discipline_id_type& discipline_id, int16_t delta)
{
    auto& relation = get_research_discipline_relation_by_research_and_discipline(research_id, discipline_id);

    FC_ASSERT(relation.votes_count + delta >= 0, "Votes amount cannot be negative");

    db_impl().modify(relation, [&](research_discipline_relation_object& r) {
        r.votes_count += delta;
    });

    return relation;
}

const research_discipline_relation_object& dbs_research_discipline_relation::get_research_discipline_relation(const research_discipline_relation_id_type& id) const
{
    try { return db_impl().get<research_discipline_relation_object, by_id>(id); }
    FC_CAPTURE_AND_RETHROW((id))
}


const dbs_research_discipline_relation::research_discipline_relation_refs_type dbs_research_discipline_relation::get_research_discipline_relations_by_research(const research_id_type& research_id) const
{
    research_discipline_relation_refs_type ret;

    const auto& idx = db_impl()
      .get_index<research_discipline_relation_index>()
      .indices()
      .get<by_research_id>();

    auto it_pair = idx.equal_range(research_id);
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

const dbs_research_discipline_relation::research_discipline_relation_refs_type dbs_research_discipline_relation::get_research_discipline_relations_by_discipline(const discipline_id_type& discipline_id) const
{
    research_discipline_relation_refs_type ret;

    const auto& idx = db_impl()
      .get_index<research_discipline_relation_index>()
      .indices()
      .get<by_discipline_id>();

    auto it_pair = idx.equal_range(discipline_id);
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

const research_discipline_relation_object& dbs_research_discipline_relation::get_research_discipline_relation_by_research_and_discipline(const research_id_type& research_id, const discipline_id_type& discipline_id) const
{
    try {
        return db_impl().get<research_discipline_relation_object, by_research_and_discipline>(boost::make_tuple(research_id, discipline_id));
    }
    FC_CAPTURE_AND_RETHROW((research_id)(discipline_id))
}

void dbs_research_discipline_relation::check_existence_by_research_and_discipline(const research_id_type& research_id, const discipline_id_type& discipline_id) const
{
    const auto& idx = db_impl().get_index<research_discipline_relation_index>().indices().get<by_research_and_discipline>();

    FC_ASSERT(idx.find(std::make_tuple(research_id, discipline_id)) != idx.cend(),
              "Research discipline relation for research \"${1}\" and discipline \"${2}\" does not exist", ("1", research_id)("2", discipline_id));
}

const bool dbs_research_discipline_relation::exists_by_research_and_discipline(const research_id_type& research_id, const discipline_id_type& discipline_id) const
{
    const auto& idx = db_impl()
            .get_index<research_discipline_relation_index>()
            .indices()
            .get<by_research_and_discipline>();

    auto itr = idx.find(std::make_tuple(research_id, discipline_id));
    return itr != idx.end();
}

} //namespace chain
} //namespace deip