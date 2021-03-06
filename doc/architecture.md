# Operations
In the `deip_evaluator.hpp` defined list of evaluators.
Evaluators have an interface:

    void apply(const OperationType& op) = 0;
    int get_type() const = 0;

All evaluators should implement `void do_apply(operation)` method.
Evaluators should be applied to operations. Operations defined in `deip_operations.hpp`

Operations inherited from:

    struct base_operation
    {
        void get_required_authorities( vector<authority>& )const {}
        void get_required_active_authorities( flat_set<account_name_type>& )const {}
        void get_required_owner_authorities( flat_set<account_name_type>& )const {}

        bool is_virtual()const { return false; }
        void validate()const {}
    };

In the `protocol/deip_operations.hpp` defined structs `operations` inherited from `base_operation`.

In the `protocol/operations.hpp` declared type

    typedef fc::static_variant<X,Y,Z,...> operation; // OperationType

Where `X,Y,Z` is operations from `protocol/deip_operations.hpp`

`DECLARE_OPERATION_TYPE` macro generate for type `deip::protocol::operation` two functions in the fc namespace.

    void to_variant( const OperationType&, fc::variant& );
    void from_variant( const fc::variant&, OperationType& );

And two functions in the deip::protocol namespace

    void operation_validate( const OperationType& o );
    void operation_get_required_authorities( const OperationType& op,
                                       flat_set< account_name_type >& active,
                                       flat_set< account_name_type >& owner,
                                       vector< authority >& other );

# Notes
### Unit tests. Read doc/testing.md file

    cmake -D BUILD_DEIP_TESTNET=ON -D CMAKE_BUILD_TYPE=Debug build_dir


# TODO
1. Find Out how reflection used in deip project.
2. `FC_REFLECT_TYPENAME( deip::protocol::operation )` Describe how does this work?




