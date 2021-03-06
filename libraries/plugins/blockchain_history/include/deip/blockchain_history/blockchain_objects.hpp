#pragma once

#include <deip/chain/schema/deip_object_types.hpp>

//
// Plugins should #define their SPACE_ID's so plugins with
// conflicting SPACE_ID assignments can be compiled into the
// same binary (by simply re-assigning some of the conflicting #defined
// SPACE_ID's in a build script).
//
// Assignment of SPACE_ID's cannot be done at run-time because
// various template automagic depends on them being known at compile
// time.
//

#ifndef BLOCKCHAIN_HISTORY_SPACE_ID
#define BLOCKCHAIN_HISTORY_SPACE_ID 7
#endif

namespace deip {
namespace blockchain_history {

using namespace deip::chain;

enum blockchain_history_object_type
{
    operations_history = (BLOCKCHAIN_HISTORY_SPACE_ID << 8),
    all_account_operations_history,
    account_deip_to_deip_transfers_history,
    account_deip_to_common_tokens_transfers_history,
    filtered_not_virt_operations_history,
    filtered_virt_operations_history,
    filtered_market_operations_history,
};
}
}