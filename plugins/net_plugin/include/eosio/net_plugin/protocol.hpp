#pragma once
#include <eosio/chain/block.hpp>
#include <eosio/chain/types.hpp>
#include <chrono>

namespace eosio
{
   using namespace chain;
   using namespace fc;

   static_assert(sizeof(std::chrono::system_clock::duration::rep) >= 8, "system_clock is expected to be at least 64 bits");
   typedef std::chrono::system_clock::duration::rep tstamp;

   //链大小消息
   struct chain_size_message
   {
      uint32_t last_irreversible_block_num = 0; //上一个确定的块号
      block_id_type last_irreversible_block_id; //上一个确定的块id
      uint32_t head_num = 0;                    //头块号
      block_id_type head_id;                    //头块id
   };

   // Longest domain name is 253 characters according to wikipedia.
   // Addresses include ":port" where max port is 65535, which adds 6 chars.
   // We also add our own extentions of "[:trx|:blk] - xxxxxxx", which adds 14 chars, total= 273.
   // Allow for future extentions as well, hence 384.
   constexpr size_t max_p2p_address_length = 253 + 6; //最大地址长度,端口占用6字符
   constexpr size_t max_handshake_str_length = 384;   //最大握手消息string(地址/操作系统名/代理人名字)长度

   //握手消息
   struct handshake_message
   {
      uint16_t network_version = 0;             //网络版本
      chain_id_type chain_id;                   //链id,用来标识链
      fc::sha256 node_id;                       //节点id
      chain::public_key_type key;               //密钥
      tstamp time{0};                           //时间戳
      fc::sha256 token;                         //token
      chain::signature_type sig;                //签名
      string p2p_address;                       //p2p地址
      uint32_t last_irreversible_block_num = 0; //最后一个确定下的块号
      block_id_type last_irreversible_block_id; //最后一个确定下的块id
      uint32_t head_num = 0;                    //头块号
      block_id_type head_id;                    //头块id
      string os;                                //操作系统
      string agent;                             //代理人
      int16_t generation = 0;                   //握手次数
   };

   enum go_away_reason
   {
      no_reason,       ///< no reason to go away
      self,            ///< the connection is to itself
      duplicate,       //连接多余
      wrong_chain,     ///< the peer's chain id doesn't match
      wrong_version,   //网络版本错误
      forked,          ///< the peer's irreversible blocks are different
      unlinkable,      ///< the peer sent a block we couldn't use
      bad_transaction, ///< the peer sent a transaction that failed verification
      validation,      //无效的块
      benign_other,    //良性的理由
      fatal_other,     //其他严重错误
      authentication   //验证失败
   };

   constexpr auto reason_str(go_away_reason rsn)
   {
      switch (rsn)
      {
      case no_reason:
         return "no reason";
      case self:
         return "self connect";
      case duplicate:
         return "duplicate";
      case wrong_chain:
         return "wrong chain";
      case wrong_version:
         return "wrong version";
      case forked:
         return "chain is forked";
      case unlinkable:
         return "unlinkable block received";
      case bad_transaction:
         return "bad transaction";
      case validation:
         return "invalid block";
      case authentication:
         return "authentication failure";
      case fatal_other:
         return "some other failure";
      case benign_other:
         return "some other non-fatal condition, possibly unknown block";
      default:
         return "some crazy reason";
      }
   }

   //断线消息
   struct go_away_message
   {
      go_away_message(go_away_reason r = no_reason) : reason(r), node_id() {}
      go_away_reason reason{no_reason};
      fc::sha256 node_id; //节点id
   };

   //时间消息
   struct time_message
   {
      tstamp org{0};         //起始时间
      tstamp rec{0};         //接收时间
      tstamp xmt{0};         //传输时间
      mutable tstamp dst{0}; //到达时间
   };

   //id状态
   enum id_list_modes
   {
      none,
      catch_up,          //捕获
      last_irr_catch_up, //最后一个确定
      normal             //确定很久了
   };

   //id状态转string
   constexpr auto modes_str(id_list_modes m)
   {
      switch (m)
      {
      case none:
         return "none";
      case catch_up:
         return "catch up";
      case last_irr_catch_up:
         return "last irreversible";
      case normal:
         return "normal";
      default:
         return "undefined mode";
      }
   }

   //id状态类
   template <typename T>
   struct select_ids
   {
      select_ids() : mode(none), pending(0), ids() {}
      id_list_modes mode{none}; //状态
      uint32_t pending{0};      //？
      vector<T> ids;            //id集合
      bool empty() const { return (mode == none || ids.empty()); }
   };

   using ordered_txn_ids = select_ids<transaction_id_type>; //传输id状态类集合
   using ordered_blk_ids = select_ids<block_id_type>;       //块id状态类集合

   //通知消息
   struct notice_message
   {
      notice_message() : known_trx(), known_blocks() {}
      ordered_txn_ids known_trx;    //传输id类集合
      ordered_blk_ids known_blocks; //块id类集合
   };

   //请求消息
   struct request_message
   {
      request_message() : req_trx(), req_blocks() {}
      ordered_txn_ids req_trx;    //传输id类集合
      ordered_blk_ids req_blocks; //待处理的块id集合
   };

   //同步请求消息
   struct sync_request_message
   {
      uint32_t start_block{0}; //开始块id
      uint32_t end_block{0};   //结束块id
   };

   using net_message = static_variant<handshake_message,    //握手消息
                                      chain_size_message,   //链大小消息
                                      go_away_message,      //离开消息
                                      time_message,         //时间消息
                                      notice_message,       //通知消息
                                      request_message,      //请求消息
                                      sync_request_message, //同步请求消息
                                      signed_block,         //已签名块       // which = 7
                                      packed_transaction>;  //已打包的传输块  // which = 8

} // namespace eosio

FC_REFLECT(eosio::select_ids<fc::sha256>, (mode)(pending)(ids))
FC_REFLECT(eosio::chain_size_message,
           (last_irreversible_block_num)(last_irreversible_block_id)(head_num)(head_id))
FC_REFLECT(eosio::handshake_message,
           (network_version)(chain_id)(node_id)(key)(time)(token)(sig)(p2p_address)(last_irreversible_block_num)(last_irreversible_block_id)(head_num)(head_id)(os)(agent)(generation))
FC_REFLECT(eosio::go_away_message, (reason)(node_id))
FC_REFLECT(eosio::time_message, (org)(rec)(xmt)(dst))
FC_REFLECT(eosio::notice_message, (known_trx)(known_blocks))
FC_REFLECT(eosio::request_message, (req_trx)(req_blocks))
FC_REFLECT(eosio::sync_request_message, (start_block)(end_block))

/**
 *
Goals of Network Code
1. low latency to minimize missed blocks and potentially reduce block interval
2. minimize redundant data between blocks and transactions.
3. enable rapid sync of a new node
4. update to new boost / fc



State:
   All nodes know which blocks and transactions they have
   All nodes know which blocks and transactions their peers have
   A node knows which blocks and transactions it has requested
   All nodes know when they learned of a transaction

   send hello message
   write loop (true)
      if peer knows the last irreversible block {
         if peer does not know you know a block or transactions
            send the ids you know (so they don't send it to you)
            yield continue
         if peer does not know about a block
            send transactions in block peer doesn't know then send block summary
            yield continue
         if peer does not know about new public endpoints that you have verified
            relay new endpoints to peer
            yield continue
         if peer does not know about transactions
            sends the oldest transactions that is not known by the remote peer
            yield continue
         wait for new validated block, transaction, or peer signal from network fiber
      } else {
         we assume peer is in sync mode in which case it is operating on a
         request / response basis

         wait for notice of sync from the read loop
      }


    read loop
      if hello message
         verify that peers Last Ir Block is in our state or disconnect, they are on fork
         verify peer network protocol

      if notice message update list of transactions known by remote peer
      if trx message then insert into global state as unvalidated
      if blk summary message then insert into global state *if* we know of all dependent transactions
         else close connection


    if my head block < the LIB of a peer and my head block age > block interval * round_size/2 then
    enter sync mode...
        divide the block numbers you need to fetch among peers and send fetch request
        if peer does not respond to request in a timely manner then make request to another peer
        ensure that there is a constant queue of requests in flight and everytime a request is filled
        send of another request.

     Once you have caught up to all peers, notify all peers of your head block so they know that you
     know the LIB and will start sending you real time transactions

parallel fetches, request in groups


only relay transactions to peers if we don't already know about it.

send a notification rather than a transaction if the txn is > 3mtu size.





*/
