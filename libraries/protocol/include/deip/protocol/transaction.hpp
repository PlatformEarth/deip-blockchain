#pragma once
#include <deip/protocol/operations.hpp>
#include <deip/protocol/sign_state.hpp>
#include <deip/protocol/types.hpp>

#include <numeric>

namespace deip {
namespace protocol {

struct tenant_affirmation_type
{
    external_id_type tenant;
    signature_type signature;
    extensions_type extensions;
};

struct transaction
{
    uint16_t ref_block_num = 0;
    uint32_t ref_block_prefix = 0;

    fc::time_point_sec expiration;

    vector<operation> operations;
    future_extensions extensions;
    
    digest_type digest() const;
    transaction_id_type id() const;
    void validate() const;
    digest_type sig_digest(const chain_id_type& chain_id) const;

    void set_expiration(fc::time_point_sec expiration_time);
    void set_reference_block(const block_id_type& reference_block);

    template <typename Visitor> vector<typename Visitor::result_type> visit(Visitor&& visitor)
    {
        vector<typename Visitor::result_type> results;
        for (auto& op : operations)
            results.push_back(op.visit(std::forward<Visitor>(visitor)));
        return results;
    }
    template <typename Visitor> vector<typename Visitor::result_type> visit(Visitor&& visitor) const
    {
        vector<typename Visitor::result_type> results;
        for (auto& op : operations)
            results.push_back(op.visit(std::forward<Visitor>(visitor)));
        return results;
    }

    void get_required_authorities(flat_set<account_name_type>& active,
                                  flat_set<account_name_type>& owner,
                                  vector<authority>& other) const;
};

struct signed_transaction : public transaction
{
    signed_transaction(const transaction& trx = transaction())
        : transaction(trx)
    {
    }

    const signature_type& sign(const private_key_type& key, const chain_id_type& chain_id);

    signature_type sign(const private_key_type& key, const chain_id_type& chain_id) const;

    set<public_key_type> get_required_signatures(const chain_id_type& chain_id,
                                                 const flat_set<public_key_type>& available_keys,
                                                 const authority_getter& get_active,
                                                 const authority_getter& get_owner,
                                                 const override_authority_getter& get_active_overrides) const;

    void verify_authority(const chain_id_type& chain_id,
                          const authority_getter& get_active,
                          const authority_getter& get_owner,
                          const override_authority_getter& get_active_overrides) const;

    set<public_key_type> minimize_required_signatures(const chain_id_type& chain_id,
                                                      const flat_set<public_key_type>& available_keys,
                                                      const authority_getter& get_active,
                                                      const authority_getter& get_owner,
                                                      const override_authority_getter& get_active_overrides) const;

    flat_set<public_key_type> get_signature_keys(const chain_id_type& chain_id) const;

    void verify_tenant_authority(const chain_id_type& chain_id, const authority_getter& get_tenant) const;

    vector<signature_type> signatures;
    optional<tenant_affirmation_type> tenant_signature;

    digest_type merkle_digest() const;

    void clear()
    {
        operations.clear();
        signatures.clear();
    }
};

void verify_authority(const vector<operation>& ops,
                      const flat_set<public_key_type>& sigs,
                      const authority_getter& get_active,
                      const authority_getter& get_owner,
                      const override_authority_getter& get_active_overrides,
                      const flat_set<account_name_type>& active_aprovals = flat_set<account_name_type>(),
                      const flat_set<account_name_type>& owner_approvals = flat_set<account_name_type>());

struct annotated_signed_transaction : public signed_transaction
{
    annotated_signed_transaction() {}
    annotated_signed_transaction(const signed_transaction& trx)
        : signed_transaction(trx)
        , transaction_id(trx.id())
    {
    }

    transaction_id_type transaction_id;
    uint32_t block_num = 0;
    uint32_t transaction_num = 0;
};

/// @} transactions group
}
} // deip::protocol

FC_REFLECT(deip::protocol::transaction, (ref_block_num)(ref_block_prefix)(expiration)(operations)(extensions))
FC_REFLECT_DERIVED(deip::protocol::signed_transaction, (deip::protocol::transaction), (signatures)(tenant_signature))
FC_REFLECT_DERIVED(deip::protocol::annotated_signed_transaction, (deip::protocol::signed_transaction),
    (transaction_id)(block_num)(transaction_num));

FC_REFLECT(deip::protocol::tenant_affirmation_type, (tenant)(signature)(extensions))