#include <deip/chain/database/database.hpp>
#include <deip/chain/services/dbs_discipline.hpp>
#include <deip/chain/services/dbs_expert_token.hpp>
#include <deip/chain/services/dbs_research.hpp>
#include <deip/chain/services/dbs_research_content.hpp>
#include <deip/chain/services/dbs_research_discipline_relation.hpp>
#include <deip/chain/services/dbs_research_group.hpp>
#include <deip/chain/services/dbs_review.hpp>
#include <deip/chain/services/dbs_review_vote.hpp>
#include <tuple>
#include <math.h>

namespace deip {
namespace chain {

dbs_review::dbs_review(database &db)
        : _base_type(db)
{
}

const review_object&
dbs_review::create(const research_content_id_type& research_content_id,
                   const string& content,
                   bool is_positive,
                   const account_name_type& author,
                   const std::set<discipline_id_type>& disciplines,
                   const std::map<discipline_id_type, share_type> used_expertise,
                   const int32_t& assessment_model_v,
                   const fc::optional<std::map<assessment_criteria, assessment_criteria_value>> scores)
{
    const auto& new_review = db_impl().create<review_object>([&](review_object& r) {
        const auto now = db_impl().head_block_time();

        r.research_content_id = research_content_id;
        fc::from_string(r.content, content);
        r.author = author;
        r.is_positive = is_positive;
        r.created_at = now;
        r.disciplines.insert(disciplines.begin(), disciplines.end());
        r.expertise_tokens_amount_by_discipline.insert(used_expertise.begin(), used_expertise.end());
        r.assessment_model_v = assessment_model_v;

        if (scores.valid()) 
        {
            for (auto& score : *scores)
            {
                r.scores.insert(std::make_pair(static_cast<uint16_t>(score.first), score.second));
            }
        }
    });

    return new_review;
}

const review_object& dbs_review::get(const review_id_type &id) const
{
    try
    {
        return db_impl().get<review_object, by_id>(id);
    }
    FC_CAPTURE_AND_RETHROW((id))
}

const dbs_review::review_optional_ref_type
dbs_review::get_review_if_exists(const review_id_type &id) const
{
    review_optional_ref_type result;
    const auto& idx = db_impl()
            .get_index<review_index>()
            .indicies()
            .get<by_id>();

    auto itr = idx.find(id);
    if (itr != idx.end())
    {
        result = *itr;
    }

    return result;
}

dbs_review::review_refs_type dbs_review::get_reviews_by_research_content(const research_content_id_type &research_content_id) const
{
    review_refs_type ret;

    auto it_pair = db_impl().get_index<review_index>().indicies().get<by_research_content>().equal_range(research_content_id);
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

dbs_review::review_refs_type dbs_review::get_author_reviews(const account_name_type &author) const
{
    review_refs_type ret;

    auto it_pair = db_impl().get_index<review_index>().indicies().get<by_author>().equal_range(author);
    auto it = it_pair.first;
    const auto it_end = it_pair.second;
    while (it != it_end)
    {
        ret.push_back(std::cref(*it));
        ++it;
    }

    return ret;
}

const std::map<discipline_id_type, share_type> dbs_review::get_eci_weight(const review_id_type& review_id) const
{
    const dbs_review_vote& review_votes_service = db_impl().obtain_service<dbs_review_vote>();

    const review_object& review = get(review_id);
    const auto& research_content_reviews = get_reviews_by_research_content(review.research_content_id);
    const auto& research_content_reviews_votes = review_votes_service.get_review_votes_by_researh_content(review.research_content_id);

    std::map<discipline_id_type, share_type> review_weight_by_discipline;
    for (discipline_id_type discipline_id : review.disciplines)
    {
        std::vector<std::reference_wrapper<const review_object>> research_content_reviews_for_discipline;
        std::copy_if(research_content_reviews.begin(), research_content_reviews.end(), std::back_inserter(research_content_reviews_for_discipline),
            [=](const review_object& rw) { 
                return rw.disciplines.find(discipline_id) != rw.disciplines.end();
            });

        std::vector<std::reference_wrapper<const review_vote_object>> research_content_reviews_votes_for_discipline;
        std::copy_if(research_content_reviews_votes.begin(), research_content_reviews_votes.end(), std::back_inserter(research_content_reviews_votes_for_discipline),
            [=](const review_vote_object& rw_vote) { 
                return rw_vote.discipline_id == discipline_id;
            });

        const double Cea = (double) DEIP_REVIEWER_INFLUENCE_FACTOR;
        const double Cva = (double) DEIP_CURATOR_INFLUENCE_FACTOR;
        const double n = (double) research_content_reviews_for_discipline.size();

        const double Er = (double) review.expertise_tokens_amount_by_discipline.at(discipline_id).value;

        const double Er_avg = (double) (std::accumulate(research_content_reviews_for_discipline.begin(), research_content_reviews_for_discipline.end(), 0,
            [&](int64_t acc, const review_object& rw) {
                const int64_t rw_Er = rw.expertise_tokens_amount_by_discipline.at(discipline_id).value;
                return acc + rw_Er;
            }) / research_content_reviews_for_discipline.size());

        const double Vr = (double) std::accumulate(research_content_reviews_votes_for_discipline.begin(), research_content_reviews_votes_for_discipline.end(), 0,
            [&](int64_t acc, const review_vote_object& rw_vote) {
                return rw_vote.review_id == review.id ? acc + rw_vote.weight : acc;
            });

        const double Vi = (double) std::accumulate(research_content_reviews_votes_for_discipline.begin(), research_content_reviews_votes_for_discipline.end(), 0,
            [&](int64_t acc, const review_vote_object& rw_vote) {
                return acc + rw_vote.weight;
            });

        /* 
        
        Original Formula
        
        const double Cr = (1 / n) * Cea * (Er / Er_avg) + Cva * (1 - 1 / n) * (Vr / (Vi != 0 ? Vi : 1));
        
        */

        const double review_votes_count = (double) std::count_if(research_content_reviews_votes_for_discipline.begin(), research_content_reviews_votes_for_discipline.end(),
            [&](const review_vote_object& rw_vote) {
                return rw_vote.review_id == review.id;
            });

        const double Cr = (1 / n) * Cea * (Er_avg / Er) + Cva * (1 - 1 / n);

        const double mr = (double)review.is_positive ? 1 : -1;

        const int64_t review_weight = std::round(mr * Cr * Er) + (review.is_positive ? review_votes_count * DEIP_CURATOR_INFLUENCE_BONUS : -review_votes_count * DEIP_CURATOR_INFLUENCE_BONUS);

        review_weight_by_discipline[discipline_id] = share_type(review_weight);
    }

    return review_weight_by_discipline;
}

} //namespace chain
} //namespace deip