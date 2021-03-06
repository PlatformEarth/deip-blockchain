#include <deip/research_content_reference_history/research_content_reference_history_api.hpp>
#include <deip/research_content_reference_history/research_content_reference_history_plugin.hpp>
#include <deip/research_content_reference_history/research_content_reference_history_object.hpp>
#include <deip/app/api_context.hpp>
#include <deip/app/application.hpp>
#include <deip/research_content_reference_history/research_content_reference_operation_object.hpp>
#include <map>

namespace deip {
namespace research_content_references_history {

namespace detail {
class research_content_reference_history_api_impl
{
public:
    deip::app::application& _app;

public:
    research_content_reference_history_api_impl(deip::app::application& app)
        : _app(app)
    {
    }

    template <typename history_object_type>
    std::vector<applied_research_content_reference_operation>
    get_content_references(const research_content_id_type& research_content_id) const
    {
        std::vector<applied_research_content_reference_operation> result;

        const auto db = _app.chain_database();

        const auto& idx = db->get_index<research_content_reference_operations_history_index>()
                              .indices()
                              .get<by_research_content>()
                              .equal_range(research_content_id);

        auto it = idx.first;
        const auto it_end = idx.second;
        while (it != it_end)
        {
            result.push_back(db->get(it->op));
            ++it;
        }

        return result;
    }


    template <typename history_object_type>
    std::vector<applied_research_content_reference_operation>
    get_content_references2(const external_id_type& research_content_external_id) const
    {
        std::vector<applied_research_content_reference_operation> result;
        const auto db = _app.chain_database();

        const auto& idx = db->get_index<research_content_reference_operations_history_index>()
            .indices()
            .get<by_research_content_external_id>()
            .equal_range(research_content_external_id);

        auto it = idx.first;
        const auto it_end = idx.second;
        while (it != it_end)
        {
            result.push_back(db->get(it->op));
            ++it;
        }

        return result;
    }

    template <typename history_object_type>
    std::vector<applied_research_content_reference_operation>
    get_contents_refer_to_content(const research_content_id_type& research_content_id) const
    {
        std::vector<applied_research_content_reference_operation> result;

        const auto db = _app.chain_database();

        const auto& idx = db->get_index<research_content_reference_operations_history_index>()
          .indices()
          .get<by_research_content_reference>()
          .equal_range(research_content_id);

        auto it = idx.first;
        const auto it_end = idx.second;
        while (it != it_end)
        {
            result.push_back(db->get(it->op));
            ++it;
        }

        return result;
    }


    template <typename history_object_type>
    std::vector<applied_research_content_reference_operation>
    get_contents_refer_to_content2(const external_id_type& research_content_external_id) const
    {
        std::vector<applied_research_content_reference_operation> result;

        const auto db = _app.chain_database();

        const auto& idx = db->get_index<research_content_reference_operations_history_index>()
          .indices()
          .get<by_research_content_reference_external_id>()
          .equal_range(research_content_external_id);

        auto it = idx.first;
        const auto it_end = idx.second;
        while (it != it_end)
        {
            result.push_back(db->get(it->op));
            ++it;
        }

        return result;
    }
};
} // namespace detail

research_content_reference_history_api::research_content_reference_history_api(const deip::app::api_context& ctx)
    : _impl(new detail::research_content_reference_history_api_impl(ctx.app))
{
}
research_content_reference_history_api::~research_content_reference_history_api()
{
}

void research_content_reference_history_api::on_api_startup() {
}

std::vector<applied_research_content_reference_operation>
research_content_reference_history_api::get_content_references(const research_content_id_type& research_content_id) const
{
    const auto db = _impl->_app.chain_database();
    return db->with_read_lock([&]() {
        return _impl->get_content_references<research_content_reference_operations_history_object>(research_content_id);
    });
}

std::vector<applied_research_content_reference_operation>
research_content_reference_history_api::get_content_references2(const external_id_type& research_content_external_id) const
{
    const auto db = _impl->_app.chain_database();
    return db->with_read_lock([&]() {
        return _impl->get_content_references2<research_content_reference_operations_history_object>(research_content_external_id);
    });
}

std::vector<applied_research_content_reference_operation>
research_content_reference_history_api::get_contents_refer_to_content(const research_content_id_type& research_content_id) const
{
    const auto db = _impl->_app.chain_database();
    return db->with_read_lock([&]() {
        return _impl->get_contents_refer_to_content<research_content_reference_operations_history_object>(research_content_id);
    });
}


std::vector<applied_research_content_reference_operation>
research_content_reference_history_api::get_contents_refer_to_content2(const external_id_type& research_content_external_id) const
{
    const auto db = _impl->_app.chain_database();
    return db->with_read_lock([&]() {
        return _impl->get_contents_refer_to_content2<research_content_reference_operations_history_object>(research_content_external_id);
    });
}

} // namespace research_content_references_history
} // namespace deip