#pragma once

#include <deip/protocol/types.hpp>
#include <deip/chain/deip_object_types.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <deip/chain/util/asset.hpp>

namespace deip{
namespace chain{

using deip::protocol::asset;

class vesting_contract_object : public object<vesting_contract_object_type, vesting_contract_object>
{

    vesting_contract_object() = delete;

public:

    template <typename Constructor, typename Allocator>
    vesting_contract_object(Constructor &&c, allocator<Allocator> a)
    {
        c(*this);
    }

    vesting_contract_id_type id;
    account_name_type owner;
    asset balance = asset(0, DEIP_SYMBOL);
    asset withdrawn = asset(0, DEIP_SYMBOL);
    uint32_t vesting_cliff_seconds = 0;
    uint32_t vesting_duration_seconds = 0;
    uint32_t period_duration_seconds = 0;
    time_point_sec start_timestamp;
};

struct by_owner;

typedef multi_index_container<vesting_contract_object,
                indexed_by<ordered_unique<tag<by_id>,
                            member<vesting_contract_object,
                                    vesting_contract_id_type,
                                    &vesting_contract_object::id>>,                        
                        ordered_non_unique<tag<by_owner>,
                                        member<vesting_contract_object,
                                                account_name_type,
                                                &vesting_contract_object::owner>>>,

                        allocator<vesting_contract_object>>
                        vesting_contract_index;
}
}

FC_REFLECT(deip::chain::vesting_contract_object,
                        (id)(owner)(balance)(vesting_cliff_seconds)(vesting_duration_seconds)
                        (start_timestamp)(period_duration_seconds)(withdrawn)
            )

CHAINBASE_SET_INDEX_TYPE(deip::chain::vesting_contract_object, deip::chain::vesting_contract_index)