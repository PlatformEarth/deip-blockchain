#pragma once

#include "dbs_base_impl.hpp"

#include <vector>
#include <set>
#include <functional>

#include <deip/chain/schema/deip_object_types.hpp>
#include <deip/chain/schema/research_token_sale_object.hpp>
#include <deip/chain/schema/research_object.hpp>

namespace deip {
namespace chain {
class dbs_research_token_sale : public dbs_base
{
    friend class dbservice_dbs_factory;

    dbs_research_token_sale() = delete;

protected:
    explicit dbs_research_token_sale(database& db);

public:
    using research_token_sale_refs_type = std::vector<std::reference_wrapper<const research_token_sale_object>>;
    using research_token_sale_optional_ref_type = fc::optional<std::reference_wrapper<const research_token_sale_object>>;

    const research_token_sale_object& create_research_token_sale(const external_id_type& external_id,
                                                                 const research_object& research,
                                                                 const flat_set<asset>& security_tokens_on_sale,
                                                                 const fc::time_point_sec& start_time,
                                                                 const fc::time_point_sec& end_time,
                                                                 const asset& soft_cap,
                                                                 const asset& hard_cap);

    const research_token_sale_object& get_research_token_sale_by_id(const research_token_sale_id_type& id) const;

    const research_token_sale_object& get_research_token_sale(const external_id_type& external_id) const;

    const research_token_sale_refs_type get_research_token_sales_by_research(const external_id_type& research_external_id) const;

    const research_token_sale_optional_ref_type get_research_token_sale_if_exists(const external_id_type& external_id) const;

    const research_token_sale_optional_ref_type get_research_token_sale_if_exists(const research_token_sale_id_type& id) const;

    const research_token_sale_refs_type get_by_research_id(const research_id_type &research_id) const;

    const research_token_sale_object& collect_funds(const research_token_sale_id_type& id, const asset& amount);

    const research_token_sale_object& update_status(const research_token_sale_id_type &id,
                                                    const research_token_sale_status& status);

    research_token_sale_refs_type get_by_research_id_and_status(const research_id_type& research_id,
                                                                const research_token_sale_status& status) const;


    using research_token_sale_contribution_refs_type = std::vector<std::reference_wrapper<const research_token_sale_contribution_object>>;
    using research_token_sale_contribution_optional_ref_type = fc::optional<std::reference_wrapper<const research_token_sale_contribution_object>>;

    const research_token_sale_contribution_object& contribute(const research_token_sale_id_type& research_token_sale_id,
                                                              const account_name_type& owner,
                                                              const fc::time_point_sec& contribution_time,
                                                              const asset& amount);

    const research_token_sale_contribution_optional_ref_type get_research_token_sale_contribution_if_exists(const research_token_sale_contribution_id_type& id) const;

    const research_token_sale_contribution_refs_type get_research_token_sale_contributions_by_research_token_sale(const external_id_type& token_sale_external_id) const;

    const research_token_sale_contribution_refs_type get_research_token_sale_contributions_by_research_token_sale_id(const research_token_sale_id_type& research_token_sale_id) const;

    const research_token_sale_contribution_object& get_research_token_sale_contribution_by_contributor_and_research_token_sale_id(const account_name_type& owner, const research_token_sale_id_type& research_token_sale_id) const;

    const research_token_sale_contribution_optional_ref_type get_research_token_sale_contribution_by_contributor_and_research_token_sale_id_if_exists(const account_name_type& owner, const research_token_sale_id_type& research_token_sale_id) const;

    const research_token_sale_contribution_refs_type get_research_token_sale_contributions_by_contributor(const account_name_type& owner) const;

    void finish_research_token_sale(const research_token_sale_id_type& research_token_sale_id);

    void refund_research_token_sale(const research_token_sale_id_type research_token_sale_id);

    void process_research_token_sales();
};

} // namespace chain
} // namespace deip
