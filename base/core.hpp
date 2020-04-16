
#ifndef _CORE_
#define _CORE_

#include <eosio/eosio.hpp>
#include <vector>
#include <map>
#include <string>

#include "define.hpp"

using namespace eosio;
using namespace std;

namespace BASE
{

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] GameOwner
{
    name owner;

    uint64_t primary_key() const
    {
        return owner.value;    
    }
};

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] Game
{
    uint32_t gameId;
    name     gameOwner;
    string   gameName;
    string   gameCompany;
    string   gameDescription;

    uint64_t primary_key() const
    {
        return gameId;
    }

    uint64_t by_owner() const
    {
        return gameOwner.value;
    }
};

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] Composer
{
    uint32_t     gameId;
    vector<name> authComposers;

    uint64_t primary_key() const
    {
        return gameId;
    }
};

struct Property
{
    uint32_t limit;
    string   instruction;
};

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] NFTItemMetaPool
{
    uint32_t              gameId;
    map<string, Property> metaPool;

    uint64_t primary_key() const
    {
        return gameId;
    }
};

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] NFTItemPropertyPool
{
    uint32_t              gameId;
    map<string, Property> propertyPool;

    uint64_t primary_key() const
    {
        return gameId;
    }
};

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] NFTItemMeta
{
    uint32_t            itemId;
    uint32_t            gameId;
    string              description;
    name                composer;
    map<string, string> metadata;

    uint64_t primary_key() const
    {
        return static_cast<uint64_t>(gameId) << 32 | itemId;
    }

    uint64_t by_itemId() const
    {
        return itemId;
    }

    uint64_t by_gameId() const
    {
        return gameId;
    }

    uint64_t by_composer() const
    {
        return composer.value;
    }
};

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] NFTItem
{
    uint32_t            uniqueId;
    uint32_t            itemId;
    uint32_t            gameId;
    name                belongTo;
    map<string, string> properties;

    uint64_t primary_key() const
    {
        return static_cast<uint64_t>(gameId) << 32 | uniqueId;
    }

    uint64_t by_uniqueId() const
    {
        return uniqueId;
    }

    uint64_t by_itemId() const
    {
        return itemId;
    }

    uint64_t by_gameId() const
    {
        return gameId;
    }

    uint64_t by_belongTo() const
    {
        return belongTo.value;
    }
};

} // namespace BASE

namespace MI
{

typedef multi_index<"gameowner"_n, BASE::GameOwner> GameOwner;

typedef multi_index<"game"_n, BASE::Game,
            indexed_by<"owner"_n, const_mem_fun<BASE::Game, uint64_t, &BASE::Game::by_owner>>
        > Game;

typedef multi_index<"composer"_n, BASE::Composer> Composer;

typedef multi_index<"metapool"_n, BASE::NFTItemMetaPool> NFTItemMetaPool;

typedef multi_index<"propertypool"_n, BASE::NFTItemPropertyPool> NFTItemPropertyPool;

typedef multi_index<"nftmeta"_n, BASE::NFTItemMeta,
            indexed_by<"itemid"_n, const_mem_fun<BASE::NFTItemMeta, uint64_t, &BASE::NFTItemMeta::by_itemId>>,
            indexed_by<"gameid"_n, const_mem_fun<BASE::NFTItemMeta, uint64_t, &BASE::NFTItemMeta::by_gameId>>,
            indexed_by<"composer"_n, const_mem_fun<BASE::NFTItemMeta, uint64_t, &BASE::NFTItemMeta::by_composer>>
        > NFTItemMeta;

typedef multi_index<"nft"_n, BASE::NFTItem,
            indexed_by<"uniqueid"_n, const_mem_fun<BASE::NFTItem, uint64_t, &BASE::NFTItem::by_uniqueId>>,
            indexed_by<"itemid"_n, const_mem_fun<BASE::NFTItem, uint64_t, &BASE::NFTItem::by_itemId>>,
            indexed_by<"gameid"_n, const_mem_fun<BASE::NFTItem, uint64_t, &BASE::NFTItem::by_gameId>>,
            indexed_by<"belongto"_n, const_mem_fun<BASE::NFTItem, uint64_t, &BASE::NFTItem::by_itemId>>
        > NFTItem;

} // namespace MI

#endif
