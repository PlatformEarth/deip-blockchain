#include <deip/chain/database/database.hpp>
#include <deip/protocol/authority.hpp>
#include <deip/chain/schema/proposal_object.hpp>
#include <deip/chain/schema/account_object.hpp>

namespace deip {
namespace chain {

bool proposal_object::is_authorized_to_execute(chainbase::database& db) const
{
    auto get_active = [&](const string& name) { 
        return authority(db.get<account_authority_object, by_account>(name).active); 
    };
    
    auto get_owner = [&](const string& name) { 
        return authority(db.get<account_authority_object, by_account>(name).owner); 
    };

    auto get_active_overrides = [&](const string& name, const uint16_t& op_tag) {
        fc::optional<authority> result;
        const auto& auth = db.get<account_authority_object, by_account>(name);
        if (auth.active_overrides.find(op_tag) != auth.active_overrides.end())
        {
            result = auth.active_overrides.at(op_tag);
        }
        return result;
    };

    try
    {
        verify_authority(
          proposed_transaction.operations, 
          available_key_approvals, 
          get_active, 
          get_owner, 
          get_active_overrides,
          available_active_approvals, 
          available_owner_approvals
        );
   } 
   catch ( const fc::exception& e )
   {
      return false;
   }

   return true;
}



}
} // deip::chain
