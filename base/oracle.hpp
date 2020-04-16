
#ifndef _ORACLE_
#define _ORACLE_

#include <eosio/eosio.hpp>

#include "define.hpp"

using namespace eosio;
using namespace std;

namespace BASE
{

struct [[ eosio::table, eosio::contract(_CONTRACT_NAME_) ]] OracleChannel
{
    uint32_t gameId;
    name     contract;
    name     action;

    uint64_t primary_key() const
    {
        return gameId;
    }

    uint64_t by_contract() const
    {
        return contract.value;
    }
};

} // namespace BASE

namespace MI
{

typedef multi_index<"channel"_n, BASE::OracleChannel,
            indexed_by<"contract"_n, const_mem_fun<BASE::OracleChannel, uint64_t, &BASE::OracleChannel::by_contract>>
        > OracleChannel;
    
} // namespace MI

namespace MSG
{

struct ItemTransfer
{
    enum
    {
        TYPE = "transfer"_n.value
    };

    uint32_t itemId;
    uint32_t uniqueId;
    name     from;
    name     to;
};

} // namespace MSG

#endif
