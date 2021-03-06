/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 */
#pragma once

#include <deip/chain/schema/global_property_object.hpp>
#include <deip/chain/hardfork.hpp>
#include <deip/chain/schema/node_property_object.hpp>
#include <deip/chain/database/fork_database.hpp>
#include <deip/chain/block_log.hpp>
#include <deip/chain/operation_notification.hpp>

#include <deip/protocol/protocol.hpp>

#include <deip/chain/dbservice.hpp>
#include <deip/chain/genesis_state.hpp>

#include <fc/signals.hpp>
#include <fc/shared_string.hpp>
#include <fc/log/logger.hpp>

#include <map>
#include <memory>

namespace deip {
namespace chain {

using deip::protocol::asset;
using deip::protocol::authority;
using deip::protocol::operation;
using deip::protocol::price;
using deip::protocol::signed_transaction;

class database_impl;
struct genesis_state_type;

namespace util {
}

/**
 *   @class database
 *   @brief tracks the blockchain state in an extensible manner
 */
class database : public chainbase::database, public dbservice
{

public:
    database();
    virtual ~database();

    bool is_producing() const override
    {
        return _is_producing;
    }

    bool _log_hardforks = true;

    enum validation_steps
    {
        skip_nothing = 0,
        skip_witness_signature = 1 << 0, ///< used while reindexing
        skip_transaction_signatures = 1 << 1, ///< used by non-witness nodes
        skip_transaction_dupe_check = 1 << 2, ///< used while reindexing
        skip_fork_db = 1 << 3, ///< used while reindexing
        skip_block_size_check = 1 << 4, ///< used when applying locally generated transactions
        skip_tapos_check = 1 << 5, ///< used while reindexing -- note this skips expiration check as well
        skip_authority_check = 1 << 6, ///< used while reindexing -- disables any checking of authority on transactions
        skip_merkle_check = 1 << 7, ///< used while reindexing
        skip_undo_history_check = 1 << 8, ///< used while reindexing
        skip_witness_schedule_check = 1 << 9, ///< used while reindexing
        skip_validate = 1 << 10, ///< used prior to checkpoint, skips validate() call on transaction
        skip_validate_invariants = 1 << 11, ///< used to skip database invariant check on block application
        skip_undo_block = 1 << 12, ///< used to skip undo db on reindex
        skip_block_log = 1 << 13 ///< used to skip block logging on reindex
    };

    /**
     * @brief Open a database, creating a new one if necessary
     *
     * Opens a database in the specified directory. If no initialized database is found the database
     * will be initialized with the default state.
     *
     * @param data_dir Path to open or create database in
     */
    void open(const fc::path& data_dir,
              const fc::path& shared_mem_dir,
              uint64_t shared_file_size,
              uint32_t chainbase_flags,
              const genesis_state_type& genesis_state);

    /**
     * @brief Rebuild object graph from block history and open detabase
     *
     * This method may be called after or instead of @ref database::open, and will rebuild the object graph by
     * replaying blockchain history. When this method exits successfully, the database will be open.
     */
    void reindex(const fc::path& data_dir,
                 const fc::path& shared_mem_dir,
                 uint64_t shared_file_size,
                 const genesis_state_type& genesis_state);

    /**
     * @brief wipe Delete database from disk, and potentially the raw chain as well.
     * @param include_blocks If true, delete the raw chain as well as the database.
     *
     * Will close the database before wiping. Database will be closed when this function returns.
     */
    void wipe(const fc::path& data_dir, const fc::path& shared_mem_dir, bool include_blocks);
    void close();

    time_point_sec get_genesis_time() const override;

    //////////////////// db_block.cpp ////////////////////

    /**
     *  @return true if the block is in our fork DB or saved to disk as
     *  part of the official chain, otherwise return false
     */
    bool is_known_block(const block_id_type& id) const;
    bool is_known_transaction(const transaction_id_type& id) const;
    fc::sha256 get_pow_target() const;
    uint32_t get_pow_summary_target() const;
    block_id_type find_block_id_for_num(uint32_t block_num) const;
    block_id_type get_block_id_for_num(uint32_t block_num) const;
    optional<signed_block> fetch_block_by_id(const block_id_type& id) const;
    optional<signed_block> fetch_block_by_number(uint32_t num) const;
    template <typename T>
    void get_blocks_history_by_number(std::map<uint32_t, T>& result, uint32_t block_num, uint32_t limit) const
    {
        static const uint32_t max_history_depth = 100;
        FC_ASSERT(limit > 0, "Limit must be greater than zero");
        FC_ASSERT(limit <= max_history_depth, "Limit of ${l} is greater than maxmimum allowed ${2}",
                  ("l", limit)("2", max_history_depth));
        FC_ASSERT(block_num >= limit, "From must be greater than limit");
        try
        {
            uint32_t head_block_number = head_block_num();
            if (block_num > head_block_number)
            {
                block_num = head_block_number;
            }
            uint32_t from_block_num = (block_num > limit) ? block_num - limit : 0;
            result.clear();
            optional<signed_block> b;
            while (from_block_num != block_num)
            {
                b = fetch_block_by_number(block_num);
                if (b.valid())
                    result[block_num] = *b; // conver from signed_block to type T
                --block_num;
            }
        }
        FC_LOG_AND_RETHROW()
    }
    const signed_transaction get_recent_transaction(const transaction_id_type& trx_id) const;
    std::vector<block_id_type> get_block_ids_on_fork(block_id_type head_of_fork) const;

    chain_id_type get_chain_id() const;

    const witness_object& get_witness(const account_name_type& name) const override;
    const witness_object* find_witness(const account_name_type& name) const;

    const account_object& get_account(const account_name_type& name) const override;
    const account_object* find_account(const account_name_type& name) const;

    const dynamic_global_property_object& get_dynamic_global_properties() const override;
    const node_property_object& get_node_properties() const;
    const witness_schedule_object& get_witness_schedule_object() const override;
    const hardfork_property_object& get_hardfork_property_object() const;

    /**
     *  Deducts fee from the account and the share supply
     */
    void pay_fee(const account_object& a, asset fee);

    /**
     *  Calculate the percent of block production slots that were missed in the
     *  past 128 blocks, not including the current block.
     */
    uint32_t witness_participation_rate() const;

    void add_checkpoints(const flat_map<uint32_t, block_id_type>& checkpts);
    void set_tenant(const string& tenant);
    fc::optional<external_id_type> get_tenant();

    const flat_map<uint32_t, block_id_type> get_checkpoints() const
    {
        return _checkpoints;
    }
    bool before_last_checkpoint() const;

    bool push_block(const signed_block& b, uint32_t skip = skip_nothing);
    void push_transaction(const signed_transaction& trx, uint32_t skip = skip_nothing);
    void push_proposal(const proposal_object& proposal) override;
    void _maybe_warn_multiple_production(uint32_t height) const;
    bool _push_block(const signed_block& b);
    void _push_transaction(const signed_transaction& trx);

    signed_block generate_block(const fc::time_point_sec when,
                                const account_name_type& witness_owner,
                                const fc::ecc::private_key& block_signing_private_key,
                                uint32_t skip);
    signed_block _generate_block(const fc::time_point_sec when,
                                 const account_name_type& witness_owner,
                                 const fc::ecc::private_key& block_signing_private_key);
    
    void pop_block();
    void clear_pending();

    /**
     *  This method is used to track applied operations during the evaluation of a block, these
     *  operations should include any operation actually included in a transaction as well
     *  as any implied/virtual operations that resulted, such as filling an order.
     *  The applied operations are cleared after post_apply_operation.
     */
    void notify_pre_apply_operation(operation_notification& note);
    void notify_post_apply_operation(const operation_notification& note);

    // vops are not needed for low mem. Force will push them on low mem.
    void push_virtual_operation(const operation& op) override;
    void push_hf_operation(const operation& op);

    void notify_applied_block(const signed_block& block);
    void notify_on_pending_transaction(const signed_transaction& tx);
    void notify_on_pre_apply_transaction(const signed_transaction& tx);
    void notify_on_applied_transaction(const signed_transaction& tx);

    /**
     *  This signal is emitted for plugins to process every operation after it has been fully applied.
     */
    fc::signal<void(const operation_notification&)> pre_apply_operation;
    fc::signal<void(const operation_notification&)> post_apply_operation;

    /**
     *  This signal is emitted after all operations and virtual operation for a
     *  block have been applied but before the get_applied_operations() are cleared.
     *
     *  You may not yield from this callback because the blockchain is holding
     *  the write lock and may be in an "inconstant state" until after it is
     *  released.
     */
    fc::signal<void(const signed_block&)> applied_block;

    /**
     * This signal is emitted any time a new transaction is added to the pending
     * block state.
     */
    fc::signal<void(const signed_transaction&)> on_pending_transaction;

    /**
     * This signla is emitted any time a new transaction is about to be applied
     * to the chain state.
     */
    fc::signal<void(const signed_transaction&)> on_pre_apply_transaction;

    /**
     * This signal is emitted any time a new transaction has been applied to the
     * chain state.
     */
    fc::signal<void(const signed_transaction&)> on_applied_transaction;

    /**
     *  Emitted After a block has been applied and committed.  The callback
     *  should not yield and should execute quickly.
     */
    // fc::signal<void(const vector< graphene::db2::generic_id >&)> changed_objects;

    /** this signal is emitted any time an object is removed and contains a
     * pointer to the last value of every object that was removed.
     */
    // fc::signal<void(const vector<const object*>&)>  removed_objects;

    //////////////////// db_witness_schedule.cpp ////////////////////

    /**
     * @brief Get the witness scheduled for block production in a slot.
     *
     * slot_num always corresponds to a time in the future.
     *
     * If slot_num == 1, returns the next scheduled witness.
     * If slot_num == 2, returns the next scheduled witness after
     * 1 block gap.
     *
     * Use the get_slot_time() and get_slot_at_time() functions
     * to convert between slot_num and timestamp.
     *
     * Passing slot_num == 0 returns DEIP_NULL_WITNESS
     */
    account_name_type get_scheduled_witness(uint32_t slot_num) const;

    /**
     * Get the time at which the given slot occurs.
     *
     * If slot_num == 0, return time_point_sec().
     *
     * If slot_num == N for N > 0, return the Nth next
     * block-interval-aligned time greater than head_block_time().
     */
    fc::time_point_sec get_slot_time(uint32_t slot_num) const;

    /**
     * Get the last slot which occurs AT or BEFORE the given time.
     *
     * The return value is the greatest value N such that
     * get_slot_time( N ) <= when.
     *
     * If no such N exists, return 0.
     */
    uint32_t get_slot_at_time(fc::time_point_sec when) const;

    /** @return the sbd created and deposited to_account, may return DEIP if there is no median feed */

    void adjust_supply(const asset& delta, bool adjust_common_token = false);

    void process_common_tokens_withdrawals();
    void process_funds();

    /**
     * Rewards distribution
     */
    asset distribute_reward(const asset &reward, const share_type &expertise);

    time_point_sec head_block_time() const override;
    uint32_t head_block_num() const;
    block_id_type head_block_id() const;
    uint16_t current_trx_ref_block_num() const override;
    uint32_t current_trx_ref_block_prefix() const override;
    optional<transaction> current_proposed_trx() const override;
    void reset_current_proposed_trx() override;

    node_property_object& node_properties();

    uint32_t last_non_undoable_block_num() const;
    //////////////////// db_init.cpp ////////////////////
    
    void initialize_evaluators();

    /// Reset the object graph in-memory
    void initialize_indexes();

    void init_genesis(const genesis_state_type& genesis_state);

    /**
     *  This method validates transactions without adding it to the pending state.
     *  @throw if an error occurs
     */
    void validate_transaction(const signed_transaction& trx);

    /** when popping a block, the transactions that were removed get cached here so they
     * can be reapplied at the proper time */
    std::deque<signed_transaction> _popped_tx;

    void retally_witness_votes();

    bool has_hardfork(uint32_t hardfork) const override;

    /* For testing and debugging only. Given a hardfork
       with id N, applies all hardforks with id <= N */
    void set_hardfork(uint32_t hardfork, bool process_now = true);

    void validate_invariants() const;
    /**
     * @}
     */

    void set_flush_interval(uint32_t flush_blocks);
    void show_free_memory(bool force);

    // witness_schedule

    void update_witness_schedule();

    // index

    template <typename MultiIndexType> void add_plugin_index()
    {
        _plugin_index_signal.connect([this]() { this->add_index<MultiIndexType>(); });
    }

private:
    void _reset_virtual_schedule_time();

    void _update_median_witness_props();

    asset allocate_rewards_to_reviews(const std::vector<review_object> &reviews, const discipline_id_type &discipline_id,
                                               const asset &reward, const share_type &expertise_reward);

protected:
    // Mark pop_undo() as protected -- we do not want outside calling pop_undo(); it should call pop_block() instead
    // void pop_undo() { object_database::pop_undo(); }
    void notify_changed_objects();

    void set_producing(bool p)
    {
        _is_producing = p;
    }

    void apply_block(const signed_block& next_block, uint32_t skip = skip_nothing);
    void apply_transaction(const signed_transaction& trx, uint32_t skip = skip_nothing);
    void _apply_block(const signed_block& next_block);
    void _apply_transaction(const signed_transaction& trx);
    void apply_operation(const operation& op);

    /// Steps involved in applying a new block
    ///@{

    const witness_object& validate_block_header(uint32_t skip, const signed_block& next_block) const;
    void create_block_summary(const signed_block& next_block);

    void update_global_dynamic_data(const signed_block& b);
    void update_signing_witness(const witness_object& signing_witness, const signed_block& new_block);
    void update_last_irreversible_block();
    void clear_expired_transactions();
    void process_content_activity_windows();
    void process_header_extensions(const signed_block& next_block);

    void init_hardforks(fc::time_point_sec genesis_time);
    void process_hardforks();
    void apply_hardfork(uint32_t hardfork);
    ///@}

    void init_genesis_accounts(const genesis_state_type& genesis_state);
    void init_genesis_assets(const genesis_state_type& genesis_state);
    void init_genesis_account_balances(const genesis_state_type& genesis_state);
    void init_genesis_witnesses(const genesis_state_type& genesis_state);
    void init_genesis_witness_schedule(const genesis_state_type& genesis_state);
    void init_genesis_global_property_object(const genesis_state_type& genesis_state);
    void init_genesis_disciplines(const genesis_state_type& genesis_state);
    void init_genesis_expert_tokens(const genesis_state_type& genesis_state);
    void init_genesis_research(const genesis_state_type& genesis_state);
    void init_genesis_research_content(const genesis_state_type& genesis_state);
    void init_genesis_research_content_reviews(const genesis_state_type& genesis_state);
    void init_genesis_research_groups(const genesis_state_type& genesis_state);
    void init_genesis_research_group(const genesis_state_type::research_group_type& research_group);
    void init_genesis_vesting_balances(const genesis_state_type& genesis_state);
    void init_genesis_proposals(const genesis_state_type& genesis_state);


private:
    std::unique_ptr<database_impl> _my;

    bool _is_producing = false;

    optional<chainbase::database::session> _pending_tx_session;

    vector<signed_transaction> _pending_tx;
    fork_database _fork_db;
    fc::time_point_sec _hardfork_times[DEIP_NUM_HARDFORKS + 1];
    protocol::hardfork_version _hardfork_versions[DEIP_NUM_HARDFORKS + 1];

    block_log _block_log;

    fc::signal<void()> _plugin_index_signal;

    // TODO: move these to trx_context
    transaction_id_type _current_trx_id;
    uint16_t _current_trx_ref_block_num;
    uint32_t _current_trx_ref_block_prefix;
    optional<transaction> _current_proposed_trx;

    uint32_t _current_block_num = 0;
    uint16_t _current_trx_in_block = 0;
    uint16_t _current_op_in_trx = 0;

    flat_map<uint32_t, block_id_type> _checkpoints;
    fc::optional<external_id_type> _tenant;

    // Counts nested proposal updates
    uint32_t _push_proposal_nesting_depth = 0;

    node_property_object _node_property_object;

    uint32_t _flush_blocks = 0;
    uint32_t _next_flush_block = 0;

    uint32_t _last_free_gb_printed = 0;
    fc::time_point_sec _const_genesis_time; // should be const
};
} // namespace chain
} // namespace deip
