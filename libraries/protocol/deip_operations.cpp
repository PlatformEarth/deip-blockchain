#include <deip/protocol/deip_operations.hpp>
#include <fc/io/json.hpp>

#include <locale>

namespace deip {
namespace protocol {


void vote_for_review_operation::validate() const
{
    validate_account_name(voter);
    FC_ASSERT(weight.amount > 0 && weight.amount <= DEIP_100_PERCENT, "Weight should be in 1% to 100% range");

    validate_160_bits_hexadecimal_string(external_id);
    validate_160_bits_hexadecimal_string(review_external_id);
    validate_160_bits_hexadecimal_string(discipline_external_id);
}

void transfer_operation::validate() const
{
    try
    {
        validate_account_name(from);
        validate_account_name(to);

        FC_ASSERT(amount.amount > 0, "Amount is not specified");
        FC_ASSERT(memo.size() < DEIP_MAX_MEMO_SIZE, "Memo is too large");
        FC_ASSERT(fc::is_utf8(memo), "Memo is not UTF8");
    }
    FC_CAPTURE_AND_RETHROW((*this))
}

void transfer_to_common_tokens_operation::validate() const
{
    validate_account_name(from);
    FC_ASSERT(amount > asset(0, DEIP_SYMBOL), "Amount must be DEIP and > 0");
    if (to != account_name_type())
        validate_account_name(to);
}

void withdraw_common_tokens_operation::validate() const
{
    validate_account_name(account);

    // TODO Add Common token validation
}

void set_withdraw_common_tokens_route_operation::validate() const
{
    validate_account_name(from_account);
    validate_account_name(to_account);
    FC_ASSERT(0 <= percent && percent <= DEIP_100_PERCENT, "Percent must be valid deip percent");
}

void witness_update_operation::validate() const
{
    validate_account_name(owner);
    FC_ASSERT(url.size() > 0, "URL size must be greater than 0");
    FC_ASSERT(fc::is_utf8(url), "URL is not valid UTF8");
    FC_ASSERT(fee > asset(0, DEIP_SYMBOL), "Fee must be DEIP and cannot be negative");
    props.validate();
}

void account_witness_vote_operation::validate() const
{
    validate_account_name(account);
    validate_account_name(witness);
}

void account_witness_proxy_operation::validate() const
{
    validate_account_name(account);
    if (proxy.size())
        validate_account_name(proxy);
    FC_ASSERT(proxy != account, "Cannot proxy to self");
}

void request_account_recovery_operation::validate() const
{
    validate_account_name(recovery_account);
    validate_account_name(account_to_recover);
    new_owner_authority.validate();
}

void recover_account_operation::validate() const
{
    validate_account_name(account_to_recover);
    FC_ASSERT(!(new_owner_authority == recent_owner_authority),
              "Cannot set new owner authority to the recent owner authority");
    FC_ASSERT(!new_owner_authority.is_impossible(), "new owner authority cannot be impossible");
    FC_ASSERT(!recent_owner_authority.is_impossible(), "recent owner authority cannot be impossible");
    FC_ASSERT(new_owner_authority.weight_threshold, "new owner authority cannot be trivial");
    new_owner_authority.validate();
    recent_owner_authority.validate();
}

void change_recovery_account_operation::validate() const
{
    validate_account_name(account_to_recover);
    validate_account_name(new_recovery_account);
}

void contribute_to_token_sale_operation::validate() const
{
    validate_account_name(contributor);
    validate_160_bits_hexadecimal_string(token_sale_external_id);
    FC_ASSERT(amount.amount > 0, "Contribution must and greater than 0");
}

void create_vesting_balance_operation::validate() const
{
    FC_ASSERT(balance > asset(0, DEIP_SYMBOL), "Deposit balance must be DEIP and greater than 0");
    FC_ASSERT(vesting_duration_seconds > 0 && vesting_duration_seconds > vesting_cliff_seconds,
            "Vesting  duration must be longer than 0 & longer than cliff period");
    FC_ASSERT(vesting_cliff_seconds >= 0, "Vesting cliff period should be equal or greater than 0");
    FC_ASSERT(period_duration_seconds > 0, "Vesting withdraw periods duration should be greater than 0");
    FC_ASSERT(vesting_duration_seconds % period_duration_seconds == 0,
            "Vesting duration should contain an integer number of withdraw periods");
    validate_account_name(creator);
    validate_account_name(owner);
}

void withdraw_vesting_balance_operation::validate() const
{
    FC_ASSERT(amount > asset(0, DEIP_SYMBOL), "Withdraw amount must be DEIP and greater than 0");
    validate_account_name(owner);
}

void create_expertise_allocation_proposal_operation::validate() const
{
    validate_account_name(claimer);
    FC_ASSERT(discipline_id > 0, "Cannot use root discipline (id = 0)");
    FC_ASSERT(description.size() > 0, "Description must be specified");
    FC_ASSERT(fc::is_utf8(description), "Description is not valid UTF8 string");
}

void vote_for_expertise_allocation_proposal_operation::validate() const
{
    validate_account_name(voter);
    FC_ASSERT(voting_power == DEIP_100_PERCENT || voting_power == -DEIP_100_PERCENT, "Voting power must be -100% or +100%");
}


void create_grant_application_operation::validate() const
{
    validate_account_name(creator);
    validate_160_bits_hexadecimal_string(funding_opportunity_number);
    FC_ASSERT(application_hash.size() > 0, "Application hash must be specified");
    FC_ASSERT(fc::is_utf8(application_hash), "Application hash is not valid UTF8 string");
}

void create_review_for_application_operation::validate() const
{
    validate_account_name(author);
    FC_ASSERT(grant_application_id >= 0, "Id cant be less than a 0");
    FC_ASSERT(weight > 0 && weight <= DEIP_100_PERCENT, "Weight should be in 1% to 100% range");
    FC_ASSERT(!content.empty(), "Content cannot be empty");
}

void approve_grant_application_operation::validate() const
{
    FC_ASSERT(grant_application_id >= 0, "Application id cant be less than a 0");
    validate_account_name(approver);
}

void reject_grant_application_operation::validate() const
{
    FC_ASSERT(grant_application_id >= 0, "Application id cant be less than a 0");
    validate_account_name(rejector);
}

} // namespace deip::protocol
} // namespace deip