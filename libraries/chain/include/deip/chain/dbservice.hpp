#pragma once

#include <fc/shared_string.hpp>

#include <deip/chain/services/dbs_base_impl.hpp>
#include <deip/protocol/operations.hpp>
#include <deip/protocol/transaction.hpp>

namespace chainbase {
class database; // for _temporary_public_imp only
}

namespace deip {
namespace chain {

class dbservice : public dbservice_dbs_factory
{
    typedef dbservice_dbs_factory _base_type;

protected:
    explicit dbservice(database&);

public:
    virtual ~dbservice();

    // TODO: These methods have copied from database public methods.
    //       Most of these methods will be moved to dbs specific services

    virtual bool is_producing() const = 0;

    virtual const witness_object& get_witness(const account_name_type& name) const = 0;

    virtual const account_object& get_account(const account_name_type& name) const = 0;

    virtual const dynamic_global_property_object& get_dynamic_global_properties() const = 0;

    virtual const witness_schedule_object& get_witness_schedule_object() const = 0;

    virtual time_point_sec head_block_time() const = 0;

    virtual uint16_t current_trx_ref_block_num() const = 0;

    virtual uint32_t current_trx_ref_block_prefix() const = 0;

    virtual optional<deip::protocol::transaction> current_proposed_trx() const = 0;

    virtual void reset_current_proposed_trx() = 0;

    virtual bool has_hardfork(uint32_t hardfork) const = 0;

    virtual fc::time_point_sec get_genesis_time() const = 0;
    
    virtual void push_virtual_operation(const protocol::operation& op) = 0;

    virtual void push_proposal(const proposal_object& proposal) = 0;

    // for TODO only:
    chainbase::database& _temporary_public_impl();
};
}
}