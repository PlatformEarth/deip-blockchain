#pragma once

#include <fc/api.hpp>
#include <deip/blockchain_history/applied_operation.hpp>

namespace deip {
namespace app {
struct api_context;
}
} // namespace deip

namespace deip {
namespace blockchain_history {

namespace detail {
class account_history_api_impl;
}

class account_history_api
{
public:
    account_history_api(const deip::app::api_context& ctx);
    ~account_history_api();

    void on_api_startup();

    /**
    *  Account operations have sequence numbers from 0 to N where N is the most recent operation. This method
    *  returns operations in the range [from-limit, from]
    *
    *  @param from - the absolute sequence number, -1 means most recent, limit is the number of operations before from.
    *  @param limit - the maximum number of items that can be queried (0 to 1000], must be less than from
    */
    std::map<uint32_t, applied_operation>
    get_account_history(const std::string& account, uint64_t from, uint32_t limit) const;

    std::map<uint32_t, applied_operation>
    get_account_deip_to_deip_transfers(const std::string& account, uint64_t from, uint32_t limit) const;

    std::map<uint32_t, applied_operation>
    get_account_deip_to_common_tokens_transfers(const std::string& account, uint64_t from, uint32_t limit) const;

private:
    std::unique_ptr<detail::account_history_api_impl> _impl;
};
} // namespace blockchain_history
} // namespace deip

FC_API(deip::blockchain_history::account_history_api,
       (get_account_history)(get_account_deip_to_deip_transfers)(get_account_deip_to_common_tokens_transfers))