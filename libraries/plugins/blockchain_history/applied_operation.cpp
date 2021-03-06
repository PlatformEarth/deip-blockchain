#include <deip/blockchain_history/applied_operation.hpp>

namespace deip {
namespace blockchain_history {

applied_operation::applied_operation()
{
}

applied_operation::applied_operation(const operation_object& op_obj)
    : trx_id(op_obj.trx_id)
    , block(op_obj.block)
    , trx_in_block(op_obj.trx_in_block)
    , op_in_trx(op_obj.op_in_trx)
    , timestamp(op_obj.timestamp)
{
    // fc::raw::unpack( op_obj.serialized_op, op );     // g++ refuses to compile this as ambiguous
    op = fc::raw::unpack<operation>(op_obj.serialized_op);
}
}
}