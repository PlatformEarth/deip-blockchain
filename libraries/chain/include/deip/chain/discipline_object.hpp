#pragma once

#include <fc/fixed_string.hpp>

#include <deip/protocol/authority.hpp>
#include <deip/protocol/deip_operations.hpp>

#include <deip/chain/deip_object_types.hpp>
#include <deip/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <numeric>

namespace deip {
namespace chain {

using deip::protocol::asset;

class discipline_object : public object<discipline_object_type, discipline_object>
{
    discipline_object() = delete;

public:

    template <typename Constructor, typename Allocator>
    discipline_object(Constructor&& c, allocator<Allocator> a)
    {
        c(*this);
    }

    discipline_id_type id;
    discipline_id_type parent_id;
    discipline_name_type name;
    share_type votes_in_last_ten_weeks;
};

struct by_discipline_name;
struct by_parent_id;

typedef multi_index_container<discipline_object,
            indexed_by<ordered_unique<tag<by_id>,
                    member<discipline_object,
                           discipline_id_type,
                           &discipline_object::id>>,
            ordered_unique<tag<by_discipline_name>,
                    member<discipline_object,
                           discipline_name_type ,
                           &discipline_object::name>>,
            ordered_non_unique<tag<by_parent_id>,
                    member<discipline_object,
                           discipline_id_type,
                           &discipline_object::parent_id>>>,
        allocator<discipline_object>>
        discipline_index;
    }
}

FC_REFLECT( deip::chain::discipline_object,
            (id)(parent_id)(name)(votes_in_last_ten_weeks)
)

CHAINBASE_SET_INDEX_TYPE( deip::chain::discipline_object, deip::chain::discipline_index )

