#pragma once

#include <deip/protocol/exceptions.hpp>

#define DEIP_DECLARE_OP_BASE_EXCEPTIONS(op_name)                                                                     \
    FC_DECLARE_DERIVED_EXCEPTION(op_name##_validate_exception, deip::chain::operation_validate_exception,            \
                                 4040000 + 100 * protocol::operation::tag<protocol::op_name##_operation>::value,       \
                                 #op_name "_operation validation exception")                                           \
    FC_DECLARE_DERIVED_EXCEPTION(op_name##_evaluate_exception, deip::chain::operation_evaluate_exception,            \
                                 4050000 + 100 * protocol::operation::tag<protocol::op_name##_operation>::value,       \
                                 #op_name "_operation evaluation exception")

#define DEIP_DECLARE_OP_VALIDATE_EXCEPTION(exc_name, op_name, seqnum, msg)                                           \
    FC_DECLARE_DERIVED_EXCEPTION(                                                                                      \
        op_name##_##exc_name, deip::chain::op_name##_validate_exception,                                             \
        4040000 + 100 * protocol::operation::tag<protocol::op_name##_operation>::value + seqnum, msg)

#define DEIP_DECLARE_OP_EVALUATE_EXCEPTION(exc_name, op_name, seqnum, msg)                                           \
    FC_DECLARE_DERIVED_EXCEPTION(                                                                                      \
        op_name##_##exc_name, deip::chain::op_name##_evaluate_exception,                                             \
        4050000 + 100 * protocol::operation::tag<protocol::op_name##_operation>::value + seqnum, msg)

#define DEIP_DECLARE_INTERNAL_EXCEPTION(exc_name, seqnum, msg)                                                       \
    FC_DECLARE_DERIVED_EXCEPTION(internal_##exc_name, deip::chain::internal_exception, 4990000 + seqnum, msg)

#define DEIP_TRY_NOTIFY(signal, ...)                                                                                 \
    try                                                                                                                \
    {                                                                                                                  \
        signal(__VA_ARGS__);                                                                                           \
    }                                                                                                                  \
    catch (const deip::chain::plugin_exception& e)                                                                   \
    {                                                                                                                  \
        throw;                                                                                                         \
    }                                                                                                                  \
    catch (const fc::exception& e)                                                                                     \
    {                                                                                                                  \
        elog("Caught exception in plugin: ${e}", ("e", e.to_detail_string()));                                         \
    }                                                                                                                  \
    catch (...)                                                                                                        \
    {                                                                                                                  \
        wlog("Caught unexpected exception in plugin");                                                                 \
    }

namespace deip {
namespace chain {

FC_DECLARE_EXCEPTION(chain_exception, 4000000, "blockchain exception")
FC_DECLARE_DERIVED_EXCEPTION(database_query_exception,
                             deip::chain::chain_exception,
                             4010000,
                             "database query exception")
FC_DECLARE_DERIVED_EXCEPTION(block_validate_exception,
                             deip::chain::chain_exception,
                             4020000,
                             "block validation exception")
FC_DECLARE_DERIVED_EXCEPTION(transaction_exception,
                             deip::chain::chain_exception,
                             4030000,
                             "transaction validation exception")
FC_DECLARE_DERIVED_EXCEPTION(operation_validate_exception,
                             deip::chain::chain_exception,
                             4040000,
                             "operation validation exception")
FC_DECLARE_DERIVED_EXCEPTION(operation_evaluate_exception,
                             deip::chain::chain_exception,
                             4050000,
                             "operation evaluation exception")
FC_DECLARE_DERIVED_EXCEPTION(utility_exception, deip::chain::chain_exception, 4060000, "utility method exception")
FC_DECLARE_DERIVED_EXCEPTION(undo_database_exception,
                             deip::chain::chain_exception,
                             4070000,
                             "undo database exception")
FC_DECLARE_DERIVED_EXCEPTION(unlinkable_block_exception, deip::chain::chain_exception, 4080000, "unlinkable block")
FC_DECLARE_DERIVED_EXCEPTION(unknown_hardfork_exception,
                             deip::chain::chain_exception,
                             4090000,
                             "chain attempted to apply unknown hardfork")
FC_DECLARE_DERIVED_EXCEPTION(plugin_exception, deip::chain::chain_exception, 4100000, "plugin exception")
FC_DECLARE_DERIVED_EXCEPTION(block_log_exception, deip::chain::chain_exception, 4110000, "block log exception")

FC_DECLARE_DERIVED_EXCEPTION(transaction_expiration_exception,
                             deip::chain::transaction_exception,
                             4030100,
                             "transaction expiration exception")
FC_DECLARE_DERIVED_EXCEPTION(transaction_tapos_exception,
                             deip::chain::transaction_exception,
                             4030200,
                             "transaction tapos exception")

FC_DECLARE_DERIVED_EXCEPTION(pop_empty_chain,
                             deip::chain::undo_database_exception,
                             4070001,
                             "there are no blocks to pop")

DEIP_DECLARE_OP_BASE_EXCEPTIONS(transfer);
//   DEIP_DECLARE_OP_EVALUATE_EXCEPTION( from_account_not_whitelisted, transfer, 1, "owner mismatch" )

DEIP_DECLARE_OP_BASE_EXCEPTIONS(create_account);
DEIP_DECLARE_OP_EVALUATE_EXCEPTION(max_auth_exceeded, create_account, 1, "Exceeds max authority fan-out")
DEIP_DECLARE_OP_EVALUATE_EXCEPTION(auth_account_not_found, create_account, 2, "Auth account not found")

DEIP_DECLARE_OP_BASE_EXCEPTIONS(update_account);
DEIP_DECLARE_OP_EVALUATE_EXCEPTION(max_auth_exceeded, update_account, 1, "Exceeds max authority fan-out")
DEIP_DECLARE_OP_EVALUATE_EXCEPTION(auth_account_not_found, update_account, 2, "Auth account not found")

FC_DECLARE_DERIVED_EXCEPTION(internal_exception, deip::chain::chain_exception, 4990000, "internal exception")

DEIP_DECLARE_INTERNAL_EXCEPTION(verify_auth_max_auth_exceeded, 1, "Exceeds max authority fan-out")
DEIP_DECLARE_INTERNAL_EXCEPTION(verify_auth_account_not_found, 2, "Auth account not found")
}
} // deip::chain

#pragma once

#include <fc/exception/exception.hpp>
#include <deip/protocol/exceptions.hpp>

namespace deip {
namespace chain {
}
} // deip::chain
