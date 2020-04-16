
#ifndef _MARKET_
#define _MARKET_

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <map>

#include "define.hpp"

using namespace eosio;
using namespace std;

namespace BASE
{

struct Permission
{
    bool exchange;
    bool auction;
};

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] Grant
{
    uint32_t gameId;
    map<name, Permission> players;

    uint64_t primary_key() const
    {
        return gameId;
    }
};

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] Exchange
{
    uint32_t   gameId;
    uint32_t   uniqueId;
    uint32_t   itemId;
    name       seller;
    name       buyer;
    asset      price;
    time_point dealTime;
    bool       closed;

    uint64_t primary_key() const
    {
        return static_cast<uint64_t>(gameId) << 32 | uniqueId;
    }

    uint64_t by_gameId() const
    {
        return gameId;
    }

    uint64_t by_uniqueId() const
    {
        return uniqueId;
    }

    uint64_t by_itemId() const
    {
        return itemId;
    }

    uint64_t by_seller() const
    {
        return seller.value;
    }

    uint64_t by_buyer() const
    {
        return buyer.value;
    }

    uint64_t by_price() const
    {
        return price.amount;
    }

    uint64_t by_dealTime() const
    {
        return dealTime.elapsed.count();
    }

    uint64_t by_diffTime() const
    {
        return (current_time_point() - dealTime).count();
    }

    uint64_t by_closed() const
    {
        return closed;
    }
};

} // namespace BASE

namespace MI
{

typedef multi_index<"marketgrant"_n, BASE::Grant> Grant;

typedef multi_index<"exchange"_n, BASE::Exchange,
            indexed_by<"gameid"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_gameId>>,
            indexed_by<"uniqueid"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_uniqueId>>,
            indexed_by<"itemid"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_itemId>>,
            indexed_by<"seller"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_seller>>,
            indexed_by<"buyer"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_buyer>>,
            indexed_by<"price"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_price>>,
            indexed_by<"dealtime"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_dealTime>>,
            indexed_by<"closed"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_closed>>
        > Exchange;

typedef multi_index<"auction"_n, BASE::Exchange,
            indexed_by<"gameid"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_gameId>>,
            indexed_by<"uniqueid"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_uniqueId>>,
            indexed_by<"itemid"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_itemId>>,
            indexed_by<"seller"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_seller>>,
            indexed_by<"bidder"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_buyer>>,
            indexed_by<"price"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_price>>,
            indexed_by<"dealtime"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_dealTime>>,
            indexed_by<"difftime"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_diffTime>>,
            indexed_by<"closed"_n, const_mem_fun<BASE::Exchange, uint64_t, &BASE::Exchange::by_closed>>
        > Auction;
    
} // namespace MI

#endif
