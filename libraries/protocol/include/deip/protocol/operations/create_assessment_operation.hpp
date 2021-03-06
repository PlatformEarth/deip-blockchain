
#pragma once
#include <deip/protocol/base.hpp>
#include <deip/protocol/version.hpp>
#include <deip/protocol/asset.hpp>

namespace deip {
namespace protocol {


struct stage_phase
{
    time_point_sec start_time;
    time_point_sec end_time;
};

struct guard_fn
{
    uint16_t fn_type;
    fc::optional<string> fn; // char*
    fc::optional<string> fn_args; // char*
    extensions_type extensions;
};


struct create_application_rule 
{
    guard_fn guard;
};

struct update_application_rule
{
    guard_fn guard;
};

struct delete_application_rule
{
    guard_fn guard;
};

typedef fc::static_variant<
  create_application_rule,
  update_application_rule,
  delete_application_rule
  >
  apply_phase_option;

struct apply_phase_type : stage_phase
{
    flat_set<apply_phase_option> options;
    extensions_type extensions;
};



struct await_review_rule
{
    extensions_type extensions;
};

typedef fc::static_variant<
  await_review_rule
  > 
  await_review_phase_option;

struct await_review_phase_type : stage_phase
{
    flat_set<await_review_phase_option> options;
    extensions_type extensions;
};



struct create_review_rule
{
    guard_fn guard;
};

struct update_review_rule
{
    guard_fn guard;
};

struct delete_review_rule
{
    guard_fn guard;
};

struct create_curation_rule
{
    guard_fn guard;
};

struct delete_curation_rule
{
    guard_fn guard;
};

typedef fc::static_variant<
  create_review_rule,
  update_review_rule,
  delete_review_rule,
  create_curation_rule,
  delete_curation_rule
  > 
  review_phase_option;

struct review_phase_type : stage_phase
{
    flat_set<review_phase_option> options;
    extensions_type extensions;
};



struct auto_decision_making_rule
{
    guard_fn guard;
};

struct manual_decision_making_rule
{
    flat_set<account_name_type> decision_makers;
    extensions_type extensions;
};

typedef fc::static_variant<
  auto_decision_making_rule,
  manual_decision_making_rule
  > 
  decision_phase_option;

struct decision_phase_type : stage_phase
{
    flat_set<decision_phase_option> options;
    extensions_type extensions;
};


typedef fc::static_variant<
  apply_phase_type,
  await_review_phase_type,
  review_phase_type,
  decision_phase_type
  > 
  assessment_stage_phase;


struct contribution_request
{
    string tag;
    bool is_mandatory;
    uint16_t type;
    extensions_type extensions;
};

struct assessment_stage
{
    external_id_type external_id;
    vector<contribution_request> contributions_requests;
    flat_set<assessment_stage_phase> phases;
    extensions_type extensions;
};


struct create_assessment_operation : public entity_operation
{
    external_id_type external_id;
    account_name_type creator;
    vector<assessment_stage> stages;
    extensions_type extensions;

    string entity_id() const { return "external_id"; }
    external_id_type get_entity_id() const { return external_id; }

    void validate() const;
    void get_required_active_authorities(flat_set<account_name_type>& a) const
    {
        a.insert(creator);
    }
};


} // namespace protocol
} // namespace deip


FC_REFLECT(deip::protocol::create_assessment_operation,
  (external_id)
  (creator)
  (stages)
  (extensions)
)


FC_REFLECT(deip::protocol::assessment_stage,
  (external_id)
  (contributions_requests)
  (phases)
  (extensions)
)


FC_REFLECT(deip::protocol::contribution_request,
  (tag)
  (is_mandatory)
  (type)
  (extensions)
)


FC_REFLECT(deip::protocol::stage_phase,
  (start_time)
  (end_time)
)


FC_REFLECT(deip::protocol::guard_fn, (fn_type)(fn)(fn_args)(extensions))

FC_REFLECT(deip::protocol::create_application_rule, (guard))
FC_REFLECT(deip::protocol::update_application_rule, (guard))
FC_REFLECT(deip::protocol::delete_application_rule, (guard))

FC_REFLECT(deip::protocol::await_review_rule, (extensions))

FC_REFLECT(deip::protocol::create_review_rule, (guard))
FC_REFLECT(deip::protocol::update_review_rule, (guard))
FC_REFLECT(deip::protocol::delete_review_rule, (guard))
FC_REFLECT(deip::protocol::create_curation_rule, (guard))
FC_REFLECT(deip::protocol::delete_curation_rule, (guard))

FC_REFLECT(deip::protocol::auto_decision_making_rule, (guard))
FC_REFLECT(deip::protocol::manual_decision_making_rule, (decision_makers)(extensions))


DECLARE_STATIC_VARIANT_TYPE(deip::protocol::apply_phase_option)
FC_REFLECT_TYPENAME(deip::protocol::apply_phase_option)
FC_REFLECT_DERIVED(deip::protocol::apply_phase_type, (deip::protocol::stage_phase),
  (options)
  (extensions)
)



DECLARE_STATIC_VARIANT_TYPE(deip::protocol::await_review_phase_option)
FC_REFLECT_TYPENAME(deip::protocol::await_review_phase_option)
FC_REFLECT_DERIVED(deip::protocol::await_review_phase_type, (deip::protocol::stage_phase),
  (options)
  (extensions)
)



DECLARE_STATIC_VARIANT_TYPE(deip::protocol::review_phase_option)
FC_REFLECT_TYPENAME(deip::protocol::review_phase_option)
FC_REFLECT_DERIVED(deip::protocol::review_phase_type, (deip::protocol::stage_phase),
  (options)
  (extensions)
)



DECLARE_STATIC_VARIANT_TYPE(deip::protocol::decision_phase_option)
FC_REFLECT_TYPENAME(deip::protocol::decision_phase_option)
FC_REFLECT_DERIVED(deip::protocol::decision_phase_type, (deip::protocol::stage_phase),
  (options)
  (extensions)
)


DECLARE_STATIC_VARIANT_TYPE(deip::protocol::assessment_stage_phase)
FC_REFLECT_TYPENAME(deip::protocol::assessment_stage_phase)