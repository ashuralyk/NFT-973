
#include "base/core.hpp"
#include "base/market.hpp"
#include "base/oracle.hpp"

class [[ eosio::contract(_CONTRACT_NAME_) ]] NFT_973
    : public contract
{
    // CORE
    MI::GameOwner           m_tbGameOwner;
    MI::Game                m_tbGame;
    MI::Composer            m_tbComposer;
    MI::NFTItemMetaPool     m_tbNFTItemMetaPool;
    MI::NFTItemPropertyPool m_tbNFTItemPropertyPool;
    MI::NFTItemMeta         m_tbNFTItemMeta;
    MI::NFTItem             m_tbNFTItem;

    // MARKET
    MI::Grant    m_tbGrant;
    MI::Exchange m_tbExchange;
    MI::Auction  m_tbAuction;

    // ORACLE
    MI::OracleChannel m_tbChannel;

public:
    NFT_973( name receiver, name code, datastream<const char *> ds )
        : contract( receiver, code, ds )
        , m_tbGameOwner( receiver, receiver.value )
        , m_tbGame( receiver, receiver.value )
        , m_tbComposer( receiver, receiver.value )
        , m_tbNFTItemMetaPool( receiver, receiver.value )
        , m_tbNFTItemPropertyPool( receiver, receiver.value )
        , m_tbNFTItemMeta( receiver, receiver.value )
        , m_tbNFTItem( receiver, receiver.value )
        , m_tbGrant( receiver, receiver.value )
        , m_tbExchange( receiver, receiver.value )
        , m_tbAuction( receiver, receiver.value )
        , m_tbChannel( receiver, receiver.value )
    {}

    /*
    * CORE
    */
public:
    [[ eosio::action ]]
    void authowner( name owner, bool auth );

    [[ eosio::action ]]
    void authcomposer( uint32_t gameId, name composer, bool auth );

    [[ eosio::action ]]
    void editgame( uint32_t gameId, string gameName, string gameCompany, string gameDesc );

    [[ eosio::action ]]
    void editmeta( uint32_t gameId, string key, BASE::Property type );

    [[ eosio::action ]]
    void editprop( uint32_t gameId, string key, BASE::Property type );

    [[ eosio::action ]]
    void composemeta( uint32_t gameId, uint32_t itemId, string key, string value );

    [[ eosio::action ]]
    void composeitem( uint32_t gameId, uint32_t itemId, uint32_t uniqueId, string key, string value );

    [[ eosio::action ]]
    void giveitemto( uint32_t gameId, uint32_t uniqueId, name player );

    /*
    * MARKET
    */
public:
    [[ eosio::action ]]
    void grant( uint32_t gameId, name player, bool exchange, bool auction );

    [[ eosio::action ]]
    void sell( uint32_t gameId, uint32_t uniqueId, asset price, bool auction );

    [[ eosio::action ]]
    void withdraw( uint32_t gameId, uint32_t uniqueId, bool auction );

    [[ eosio::action ]]
    void deal( uint32_t gameId, uint32_t uniqueId );

    [[ eosio::action ]]
    void removedeal( uint32_t gameId, uint32_t uniqueId, bool auction );

    [[ eosio::action ]]
    void transfer( uint32_t gameId, uint32_t uniqueId, name to, string memo );

    [[ eosio::on_notify("eosio.token::transfer") ]]
    void market( name from, name to, asset quantity, string memo );

private:
    void buy( name buyer, uint32_t gameId, uint32_t uniqueId, asset money, bool auction );

    void transferItemTo( uint32_t gameId, uint32_t uniqueId, name player );

    /*
    * ORACLE
    */
public:
    [[ eosio::action ]]
    void channel( uint32_t gameId, name contract, name action );

private:
    template < class _MsgType >
    void emit( uint32_t gameId, const _MsgType &&msg )
    {
        auto i = m_tbChannel.find( gameId );
        if ( i != m_tbChannel.end() ) {
            action( 
                permission_level{ get_self(), "active"_n },
                i->contract,
                i->action,
                make_tuple( _MsgType::TYPE, pack(msg) )
            )
            .send();
        }
    }

    /*
    * COMMON
    */
private:
    template < class _RType = MI::Game::const_iterator, int _In = -1 >
    _RType checkGame( uint32_t gameId )
    {
        auto i = m_tbGame.find( gameId );
        check( i != m_tbGame.end(), concat("game id '", gameId, "' dosen't exist, please edit before.") );
        if      constexpr ( _In == "gameOwner"_m )       return i->gameOwner;
        else if constexpr ( _In == "gameName"_m )        return i->gameName;
        else if constexpr ( _In == "gameCompany"_m )     return i->gameCompany;
        else if constexpr ( _In == "gameDescription"_m ) return i->gameDescription;
        else {
            return i;
        }
    }

    template < class _RType = MI::Composer::const_iterator, int _In = -1 >
    _RType checkComposer( uint32_t gameId )
    {
        checkGame<>( gameId );
        auto i = m_tbComposer.find( gameId );
        check( i != m_tbComposer.end(), concat("no composer under game id '", gameId, "', please contact the game owner to edit before.") );
        if constexpr ( _In == "authComposers"_m ) return i->authComposers;
        else {
            return i;
        }
    }

    template < class _RType = MI::NFTItem::const_iterator, int _In = -1 >
    _RType checkNFTItem( uint32_t gameId, uint32_t uniqueId )
    {
        checkGame<>( gameId );
        auto i = m_tbNFTItem.find( static_cast<uint64_t>(gameId) << 32 | uniqueId );
        check( i != m_tbNFTItem.end(), concat("item unique id '", uniqueId, "' dosen't belong to game id '", gameId, "'") );
        if      constexpr ( _In == "itemId"_m )     return i->itemId;
        else if constexpr ( _In == "belongTo"_m )   return i->belongTo;
        else if constexpr ( _In == "properties"_m ) return i->properties;
        else {
            return i;
        }
    }

    template < class _RType = MI::NFTItemMeta::const_iterator, int _In = -1 >
    _RType checkNFTItemMeta( uint32_t gameId, uint32_t itemId )
    {
        checkGame<>( gameId );
        auto i = m_tbNFTItemMeta.find( static_cast<uint64_t>(gameId) << 32 | itemId );
        check( i != m_tbNFTItemMeta.end(), concat("item id '", itemId, "' dosen't exist in game id '", gameId, "', please edit item meta before.") );
        if      constexpr ( _In == "composer"_m ) return i->composer;
        else if constexpr ( _In == "metadata"_m ) return i->metadata;
        else {
            return i;
        }
    }

    template < class _RType = MI::NFTItemMetaPool::const_iterator, int _In = -1 >
    _RType checkNFTItemMetaPool( uint32_t gameId, const string &key = "" )
    {
        checkGame<>( gameId );
        auto i = m_tbNFTItemMetaPool.find( gameId );
        check( i != m_tbNFTItemMetaPool.end(), concat("meta pool is empty under game id '", gameId, "', please contact the game owner to edit before.") );
        if      constexpr ( _In == "metaPool"_m ) return i->metaPool;
        else if constexpr ( _In == "property"_m ) {
            check( i->metaPool.count(key) > 0, concat("game owner hasn't added meta key '", key, "' into meta pool.") );
            return i->metaPool.find(key)->second;
        } else {
            return i;
        }
    }

    template < class _RType = MI::NFTItemPropertyPool::const_iterator, int _In = -1 >
    _RType checkNFTItemPropertyPool( uint32_t gameId, const string &key = "" )
    {
        checkGame<>( gameId );
        auto i = m_tbNFTItemPropertyPool.find( gameId );
        check( i != m_tbNFTItemPropertyPool.end(), concat("property pool is empty under game '", gameId, "', please contact the game owner to edit before.") );
        if      constexpr ( _In == "propertyPool"_m ) return i->propertyPool;
        else if constexpr ( _In == "property"_m ) {
            check( i->propertyPool.count(key) > 0, concat("the game owner hasn't added property key '", key, "' into property pool.") );
            return i->propertyPool.find(key)->second;
        } else {
            return i;
        }
    }

    template < class _RType = MI::Grant::const_iterator, int _In = -1 >
    _RType checkMarketGrant( uint32_t gameId, name player = ""_n )
    {
        auto i = m_tbGrant.require_find( gameId, concat("game id ", gameId, " hasn't been granted to marketing.") );
        if      constexpr ( _In == "exchange"_m ) return checkMarketGrant<_C<BASE::Permission>, "player"_m>(gameId, player).exchange;
        else if constexpr ( _In == "auction"_m )  return checkMarketGrant<_C<BASE::Permission>, "player"_m>(gameId, player).auction;
        else if constexpr ( _In == "player"_m ) {
            check( i->players.count(player) > 0, concat("game id has no granted player named ", player) );
            return i->players.find(player)->second;
        } else {
            return i;
        }
    }

    template < class _First, class ..._Type >
    const char * concat( _First &&first, _Type &&... elems )
    {
        if constexpr ( is_same<decay_t<_First>, const char *>::value ) {
            if constexpr ( sizeof...(_Type) > 0 ) {
                return (string(first) + concat<_Type...>(forward<_Type>(elems)...)).c_str();
            } else {
                return first;
            }
        } else if constexpr ( is_same<decay_t<_First>, string>::value ) {
            if constexpr ( sizeof...(_Type) > 0 ) {
                return (first + concat<_Type...>(forward<_Type>(elems)...)).c_str();
            } else {
                return first.c_str();
            }
        } else if constexpr ( is_same<decay_t<_First>, name>::value || is_same<decay_t<_First>, asset>::value ) {
            if constexpr ( sizeof...(_Type) > 0 ) {
                return (first.to_string() + concat<_Type...>(forward<_Type>(elems)...)).c_str();
            } else {
                return first.to_string().c_str();
            }
        } else {
            if constexpr ( sizeof...(_Type) > 0 ) {
                return (to_string(first) + concat<_Type...>(forward<_Type>(elems)...)).c_str();
            } else {
                return to_string(first).c_str();
            }
        }
    }
};
