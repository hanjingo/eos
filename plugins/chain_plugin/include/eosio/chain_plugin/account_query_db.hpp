#pragma once
#include <eosio/chain/types.hpp>
#include <eosio/chain/block_state.hpp>
#include <eosio/chain/trace.hpp>

namespace eosio::chain_apis {
   //一个临时的索引类
   class account_query_db {
   public:

      /**
       * Instantiate a new account query DB from the given chain controller
       * The caller is expected to manage lifetimes such that this controller reference does not go stale
       * for the life of the account query DB
       * @param chain - controller to read data from
       */
      account_query_db( const class eosio::chain::controller& chain );
      ~account_query_db();

      //实现账号赋值的move功能
      account_query_db(account_query_db&&);
      account_query_db& operator=(account_query_db&&);
      /**
       * 添加一个账号查询追踪
       * @param trace
       */
      void cache_transaction_trace( const chain::transaction_trace_ptr& trace );
      /**
       * 提交块
       * @param block
       */
      void commit_block(const chain::block_state_ptr& block );

      //通过rpc查询账号时提供的参数
      struct get_accounts_by_authorizers_params{
         struct permission_level : public chain::permission_level {
         };

         std::vector<permission_level> accounts;   //账号集合
         std::vector<chain::public_key_type> keys; //密钥集合
      };

      //通过rpc查询账号时返回的结果
      struct get_accounts_by_authorizers_result{
         //结果
         struct account_result {
            chain::name                            account_name;        //账号名
            chain::name                            permission_name;     //权限名字
            fc::optional<chain::permission_level>  authorizing_account; //
            fc::optional<chain::public_key_type>   authorizing_key;     //key
            chain::weight_type                     weight;              //权重
            uint32_t                               threshold;           //门槛(用于多重签名)
         };

         std::vector<account_result> accounts; //结果集合
      };
      /**
       * 
       * @param args
       * @return 
       */
      get_accounts_by_authorizers_result get_accounts_by_authorizers( const get_accounts_by_authorizers_params& args) const;

   private:
      std::unique_ptr<struct account_query_db_impl> _impl; //实现类
   };

}

namespace fc {
   using params = eosio::chain_apis::account_query_db::get_accounts_by_authorizers_params;
   /**
    * 重载 to_variant
    * @param a
    * @param b
    */
   inline void to_variant(const params::permission_level& a, fc::variant& v) {
      if (a.permission.empty()) {
         v = a.actor.to_string();
      } else {
         v = mutable_variant_object()
            ("actor", a.actor.to_string())
            ("permission", a.permission.to_string());
      }
   }

   /**
    * 重载from_variant
    * @param v
    * @param a
    */
   inline void from_variant(const fc::variant& v, params::permission_level& a) {
      if (v.is_string()) {
         from_variant(v, a.actor);
         a.permission = {};
      } else if (v.is_object()) {
         const auto& vo = v.get_object();
         if(vo.contains("actor"))
            from_variant(vo["actor"], a.actor);
         else
            EOS_THROW(eosio::chain::invalid_http_request, "Missing Actor field");

         if(vo.contains("permission") && vo.size() == 2)
            from_variant(vo["permission"], a.permission);
         else if (vo.size() == 1)
            a.permission = {};
         else
            EOS_THROW(eosio::chain::invalid_http_request, "Unrecognized fields in account");
      }
   }
}

FC_REFLECT( eosio::chain_apis::account_query_db::get_accounts_by_authorizers_params, (accounts)(keys))
FC_REFLECT( eosio::chain_apis::account_query_db::get_accounts_by_authorizers_result::account_result, (account_name)(permission_name)(authorizing_account)(authorizing_key)(weight)(threshold))
FC_REFLECT( eosio::chain_apis::account_query_db::get_accounts_by_authorizers_result, (accounts))
