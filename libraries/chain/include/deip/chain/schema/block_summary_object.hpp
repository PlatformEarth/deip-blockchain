#pragma once
#include "deip_object_types.hpp"

namespace deip {
namespace chain {

using deip::protocol::block_id_type;

/**
 *  @brief tracks minimal information about past blocks to implement TaPOS
 *  @ingroup object
 *
 *  When attempting to calculate the validity of a transaction we need to
 *  lookup a past block and check its block hash and the time it occurred
 *  so we can calculate whether the current transaction is valid and at
 *  what time it should expire.
 */
class block_summary_object : public object<block_summary_object_type, block_summary_object>
{
public:
    template <typename Constructor, typename Allocator> block_summary_object(Constructor&& c, allocator<Allocator> a)
    {
        c(*this);
    }

    block_summary_object(){};

    id_type id;
    block_id_type block_id;
};

typedef multi_index_container<block_summary_object,
                              indexed_by<ordered_unique<tag<by_id>,
                                                        member<block_summary_object,
                                                               block_summary_object::id_type,
                                                               &block_summary_object::id>>>,
                              allocator<block_summary_object>>
    block_summary_index;
}
} // deip::chain

FC_REFLECT(deip::chain::block_summary_object, (id)(block_id))
CHAINBASE_SET_INDEX_TYPE(deip::chain::block_summary_object, deip::chain::block_summary_index)
