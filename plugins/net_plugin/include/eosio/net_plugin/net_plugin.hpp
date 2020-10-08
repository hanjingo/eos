#pragma once
#include <appbase/application.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/net_plugin/protocol.hpp>

namespace eosio
{
   using namespace appbase;

   struct connection_status
   {
      string peer;
      bool connecting = false;
      bool syncing = false;
      handshake_message last_handshake;
   };

   class net_plugin : public appbase::plugin<net_plugin>
   {
   public:
      net_plugin();
      virtual ~net_plugin();

      APPBASE_PLUGIN_REQUIRES((chain_plugin))
      virtual void set_program_options(options_description &cli, options_description &cfg) override;
      void handle_sighup() override;

      void plugin_initialize(const variables_map &options);
      void plugin_startup();
      void plugin_shutdown();

      string connect(const string &endpoint);                           //连接端点
      string disconnect(const string &endpoint);                        //断开连接
      optional<connection_status> status(const string &endpoint) const; //返回端点的状态
      vector<connection_status> connections() const;                    //返回所有的连接

   private:
      std::shared_ptr<class net_plugin_impl> my;
   };

} // namespace eosio

FC_REFLECT(eosio::connection_status, (peer)(connecting)(syncing)(last_handshake))
