
#include <tuple>
#include "nft.hpp"

using std::make_tuple;

////////////////////////////////////////////////////
// CORE ACTIONS
////////////////////////////////////////////////////

void NFT_973::authowner( name owner, bool auth )
{
    require_auth( get_self() );
    check( is_account(owner), concat("'", owner, "' must be an EOSIO account.") );

    auto i = m_tbGameOwner.find( owner.value );
    if ( auth )
    {
        check( i == m_tbGameOwner.end(), concat("'", owner, "' is already a game owner.") );
        m_tbGameOwner.emplace( get_self(), [&] (auto &v) {
            v.owner = owner;
        });
    }
    else
    {
        check( i != m_tbGameOwner.end(), concat("no owner named '", owner, "', delete auth failed.") );
        m_tbGameOwner.erase( i );
    }
}

void NFT_973::authcomposer( uint32_t gameId, name composer, bool auth )
{
    auto gameOwner = checkGame<_C<name>, "gameOwner"_m>( gameId );
    require_auth( gameOwner );
    check( is_account(composer), concat("'", composer, "' must be an EOSIO account.") );

    if ( auto i = m_tbComposer.find( gameId ); i == m_tbComposer.end() )
    {
        m_tbComposer.emplace( gameOwner, [&] (auto &v) {
            v.gameId = gameId;
            if ( auth ) {
                v.authComposers.push_back( composer );
            } else {
                check( false, concat("composer '", composer, "' dosen't exist in game id '", gameId, "', delete auth failed.") );
            }
        });
    }
    else
    {
        m_tbComposer.modify( i, gameOwner, [&] (auto &v) {
            auto f = find( v.authComposers.begin(), v.authComposers.end(), composer );
            if ( auth ) {
                check( f == v.authComposers.end(), concat("composer '", composer, "' already exists in game id '", gameId, "', add auth failed.") );
                v.authComposers.push_back( composer );
            } else {
                check( f != v.authComposers.end(), concat("composer '", composer, "' dosen't exists in game id '", gameId, "', delete auth failed.") );
                v.authComposers.erase( f );
            }
        });
    }
}

void NFT_973::editgame( uint32_t gameId, string gameName, string gameCompany, string gameDesc )
{
    name gameOwner;
    for ( const auto &i : m_tbGameOwner )
    {
        if ( has_auth(i.owner) )
        {
            gameOwner = i.owner;
            break;
        }
    }
    check( gameOwner != ""_n, "the authorization provided is not a game owner." );

    if ( auto i = m_tbGame.find(gameId); i != m_tbGame.end() )
    {
        check( (*i).gameOwner == gameOwner, concat("the game id '", gameId, "' is already in use.") );
        m_tbGame.modify( i, gameOwner, [&] (auto &v) {
            v.gameName        = gameName;
            v.gameCompany     = gameCompany;
            v.gameDescription = gameDesc;
        });
    }
    else
    {
        m_tbGame.emplace( gameOwner, [&] (auto &v) {
            v.gameId          = gameId;
            v.gameOwner       = gameOwner;
            v.gameName        = gameName;
            v.gameCompany     = gameCompany;
            v.gameDescription = gameDesc;
        });
    }
}

void NFT_973::editmeta( uint32_t gameId, string key, BASE::Property type )
{
    auto gameOwner = checkGame<_C<name>, "gameOwner"_m>( gameId );
    require_auth( gameOwner );

    if ( auto i = m_tbNFTItemMetaPool.find(gameId); i != m_tbNFTItemMetaPool.end() )
    {
        m_tbNFTItemMetaPool.modify( i, gameOwner, [&] (auto &v) {
            v.metaPool[key] = type;
        });
    }
    else
    {
        m_tbNFTItemMetaPool.emplace( gameOwner, [&] (auto &v) {
            v.gameId        = gameId;
            v.metaPool[key] = type;
        });
    }
}

void NFT_973::editprop( uint32_t gameId, string key, BASE::Property type )
{
    auto gameOwner = checkGame<_C<name>, "gameOwner"_m>( gameId );
    require_auth( gameOwner );

    if ( auto i = m_tbNFTItemPropertyPool.find(gameId); i != m_tbNFTItemPropertyPool.end() )
    {
        m_tbNFTItemPropertyPool.modify( i, gameOwner, [&] (auto &v) {
            v.propertyPool[key] = type;
        });
    }
    else
    {
        m_tbNFTItemPropertyPool.emplace( gameOwner, [&] (auto &v) {
            v.gameId            = gameId;
            v.propertyPool[key] = type;
        });
    }
}

void NFT_973::composemeta( uint32_t gameId, uint32_t itemId, string key, string value )
{
    name composer;
    for ( const auto &auth : checkComposer<vector<name>, "authComposers"_m>(gameId) )
    {
        if ( has_auth(auth) )
        {
            composer = auth;
            break;
        }
    }
    check( composer != ""_n, concat("the authorization provided is not authed as a composer by game id '", gameId, "'.") );

    auto property = checkNFTItemMetaPool<_C<BASE::Property>, "property"_m>( gameId, key );
    check( value.length() < property.limit, concat("the length of value is beyond the limit '", property.limit, "' edited by the game owner.") );

    uint64_t metaKey = static_cast<uint64_t>(gameId) << 32 | itemId;
    if ( auto i = m_tbNFTItemMeta.find(metaKey); i != m_tbNFTItemMeta.end() )
    {
        check( i->composer == composer, concat("the item id '", itemId, "' is already edited by composer '", i->composer, "'.") );
        m_tbNFTItemMeta.modify( i, composer, [&] (auto &v) {
            v.metadata[key] = value;
        });
    }
    else
    {
        m_tbNFTItemMeta.emplace( composer, [&] (auto &v) {
            v.gameId        = gameId;
            v.itemId        = itemId;
            v.composer      = composer;
            v.metadata[key] = value;
        });
    }
}

void NFT_973::composeitem( uint32_t gameId, uint32_t itemId, uint32_t uniqueId, string key, string value )
{
    auto composer = checkNFTItemMeta<name, "composer"_m>( gameId, itemId );
    require_auth( composer );

    auto property = checkNFTItemPropertyPool<_C<BASE::Property>, "property"_m>( gameId, key );
    check( value.length() <= property.limit, concat("the length of value is beyond the limit '", property.limit, "' edited by the game owner.") );

    uint64_t propertyKey = static_cast<uint64_t>(gameId) << 32 | uniqueId;
    if ( auto i = m_tbNFTItem.find(propertyKey); i != m_tbNFTItem.end() )
    {
        check( i->itemId == itemId, concat("the item id '", itemId, "' is already in use.") );
        m_tbNFTItem.modify( i, composer, [&] (auto &v) {
            v.properties[key] = value;
        });
    }
    else
    {
        m_tbNFTItem.emplace( composer, [&] (auto &v) {
            v.uniqueId        = uniqueId;
            v.gameId          = gameId;
            v.itemId          = itemId;
            v.properties[key] = value;
        });
    }
}

void NFT_973::giveitemto( uint32_t gameId, uint32_t uniqueId, name player )
{
    auto i = checkNFTItem<>( gameId, uniqueId );
    check( i->belongTo == ""_n, concat("item dose already belong to player '", i->belongTo, "'") );
    check( is_account(player), concat(player, " is not an EOS account.") );

    auto composer = checkNFTItemMeta<_C<name>, "composer"_m>( gameId, i->itemId );
    require_auth( composer );

    m_tbNFTItem.modify( i, composer, [&] (auto &v) {
        v.belongTo = player;
        emit<MSG::ItemTransfer>( gameId, MSG::ItemTransfer {
            .itemId   = i->itemId,
            .uniqueId = uniqueId,
            .from     = ""_n,
            .to       = player
        });
    });
}

////////////////////////////////////////////////////
// MARKET ACTIONS
////////////////////////////////////////////////////

void NFT_973::grant( uint32_t gameId, name player, bool exchange, bool auction )
{
    auto owner = checkGame<_C<name>, "gameOwner"_m>( gameId );
    require_auth( owner );

    if ( auto i = m_tbGrant.find(gameId); i != m_tbGrant.end() )
    {
        m_tbGrant.modify( i, owner, [&] (auto &v) {
            v.players.emplace( player, BASE::Permission {
                .exchange = exchange,
                .auction  = auction
            });
        });
    }
    else
    {
        m_tbGrant.emplace( owner, [&] (auto &v) {
            v.gameId = gameId;
            v.players.emplace( player, BASE::Permission {
                .exchange = exchange,
                .auction  = auction
            });
        });
    }

    if ( false == exchange )
    {
        auto sellerView = m_tbExchange.get_index<"seller"_n>();
        if ( auto i = sellerView.find(player.value); i != sellerView.end() )
        {
            auto end = sellerView.upper_bound( player.value );
            advance( end, 1 );
            while ( i != end )
            {
                print( i->itemId, i->uniqueId, i->price, " | " );
                if ( i->buyer == ""_n ) {
                    i = sellerView.erase( i );
                } else {
                    ++i;
                }
            }
        }
    }
}

void NFT_973::sell( uint32_t gameId, uint32_t uniqueId, asset price, bool auction )
{
    auto i = checkNFTItem<>( gameId, uniqueId );
    check( i->belongTo != ""_n, "item needs an owner" );
    check( price.amount > 0, "price must be positive" );
    require_auth( i->belongTo );

    uint64_t key = static_cast<uint64_t>(gameId) << 32 | uniqueId;
    check( m_tbAuction.find(key) == m_tbAuction.end(), "item is already on auction." );
    check( m_tbExchange.find(key) == m_tbExchange.end(), "item is already on selling." );
    if ( auction )
    {
        check( checkMarketGrant<bool, "auction"_m>(gameId, i->belongTo), concat(i->belongTo, " has no grant to auction.") );
        m_tbAuction.emplace( i->belongTo, [&] (auto &v) {
            v.gameId   = gameId;
            v.uniqueId = uniqueId;
            v.itemId   = i->itemId;
            v.seller   = i->belongTo;
            v.price    = price;
        });
    }
    else
    {
        check( checkMarketGrant<bool, "exchange"_m>(gameId, i->belongTo), concat(i->belongTo, " has no grant to exchange.") );
        m_tbExchange.emplace( i->belongTo, [&] (auto &v) {
            v.gameId   = gameId;
            v.uniqueId = uniqueId;
            v.itemId   = i->itemId;
            v.seller   = i->belongTo;
            v.price    = price;
        });
    }
}

void NFT_973::withdraw( uint32_t gameId, uint32_t uniqueId, bool auction )
{
    uint64_t key = static_cast<uint64_t>(gameId) << 32 | uniqueId;
    if ( auction )
    {
        auto i = m_tbAuction.require_find( key, "item is not on auction." );
        require_auth( i->seller );
        check( i->closed == false, "the auction has closed yet." );
        check( i->buyer == ""_n, concat("item is already auctioned by ", i->buyer, ", withdraw denied.") );
        m_tbAuction.erase( i );
    }
    else
    {
        auto i = m_tbExchange.require_find( key, "item is not on selling." );
        require_auth( i->seller );
        check( i->closed == false, "the exchange has closed yet." );
        m_tbExchange.erase( i );
    }
}

void NFT_973::buy( name buyer, uint32_t gameId, uint32_t uniqueId, asset money, bool auction )
{
    require_auth( buyer );
    auto owner = checkNFTItem<_C<name>, "belongTo"_m>( gameId, uniqueId );
    check( buyer != owner, "buyer can't be same as the item's owner" );

    uint64_t key = static_cast<uint64_t>(gameId) << 32 | uniqueId;
    if ( auction )
    {
        auto i = m_tbAuction.require_find( key, "target item is not on auction." );
        check( money.amount >= i->price.amount * 1.1, "money should be at least 10% more than item's last price." );

        if ( i->buyer != ""_n )
        {
            check( current_time_point() < i->dealTime, "the auction is already closed." );
            check( i->price.amount > 0, "last auction price must be positive." );
            action( 
                permission_level{ get_self(), "active"_n },
                "eosio.token"_n,
                "transfer"_n,
                make_tuple( get_self(), i->buyer, i->price,
                    string(concat(buyer, " has over bidded ", i->buyer, "'s auction (game: ", gameId, ", id: ", uniqueId, ")")) )
            )
            .send();
        }

        m_tbAuction.modify( i, same_payer, [&] (auto &v) {
            v.buyer    = buyer;
            v.price    = money;
            v.dealTime = current_time_point() + hours(12);
        });
    }
    else
    {
        auto i = m_tbExchange.require_find( key, "target item is not on selling." );
        check( money == i->price, concat("money should be equal to item's price (", i->price, ").") );
        m_tbExchange.modify( i, same_payer, [&] (auto &v) {
            v.buyer    = buyer;
            v.dealTime = current_time_point();
            v.closed   = true;
        });

        transferItemTo( gameId, uniqueId, buyer );
    }
}

void NFT_973::deal( uint32_t gameId, uint32_t uniqueId )
{
    require_auth( get_self() );

    uint64_t key = static_cast<uint64_t>(gameId) << 32 | uniqueId;
    auto i = m_tbAuction.require_find( key, "target item is not on auction." );
    check( current_time_point() > i->dealTime, "the item's auction hasn't timed out yet." );

    m_tbAuction.modify( i, get_self(), [&] (auto &v) {
        v.closed = true;
    });

    action( 
        permission_level{ get_self(), "active"_n },
        "eosio.token"_n,
        "transfer"_n,
        make_tuple( get_self(), i->seller, i->price,
            string(concat(i->seller, "'s auction is done by ", i->buyer, ", congratulations both of you!")) )
    )
    .send();

    transferItemTo( gameId, uniqueId, i->buyer );
}

void NFT_973::removedeal( uint32_t gameId, uint32_t uniqueId, bool auction )
{
    require_auth( get_self() );

    uint64_t key = static_cast<uint64_t>(gameId) << 32 | uniqueId;
    if ( auction )
    {
        auto i = m_tbAuction.require_find( key, "target item is not on auction." );
        check( i->closed, "only support to delete closed deal." );
        m_tbAuction.erase( i );
    }
    else
    {
        auto i = m_tbExchange.require_find( key, "target item is not on selling." );
        check( i->closed, "only support to delete closed deal." );
        m_tbExchange.erase( i );
    }
}

void NFT_973::transfer( uint32_t gameId, uint32_t uniqueId, name to, string memo )
{
    auto belongTo = checkNFTItem<_C<name>, "belongTo"_m>( gameId, uniqueId );
    require_auth( belongTo );

    transferItemTo( gameId, uniqueId, to );
}

void NFT_973::market( name from, name to, asset quantity, string memo /*= "buy,[gameId],[uniqueId],[auction]"*/ )
{
    if ( from == get_self() || to != get_self() || quantity.symbol != symbol("EOS", 4) )
    {
        return;
    }

    vector<string> command;
    size_t s = 0;
    for ( size_t e = memo.find(','); e != string::npos; s = e + 1, e = memo.find(',', s) )
    {
        command.emplace_back( memo.substr(s, e - s) );
        // print( s, ':', memo.substr(s, e - s), ',' );
    }
    // print( s, ':', memo.substr(s) );
    command.emplace_back( memo.substr(s) );

    if ( command[0] == "buy" )
    {
        check( command.size() >= 4, "invalid 'buy' command params." );

        uint32_t gameId   = static_cast<uint32_t>( atoi(command[1].c_str()) );
        uint32_t uniqueId = static_cast<uint32_t>( atoi(command[2].c_str()) );
        bool     auction  = static_cast<bool>( atoi(command[3].c_str()) );

        buy( from, gameId, uniqueId, quantity, auction );
    }
}

void NFT_973::transferItemTo( uint32_t gameId, uint32_t uniqueId, name player )
{
    auto i = checkNFTItem<>( gameId, uniqueId );
    m_tbNFTItem.modify( i, same_payer, [&] (auto &v) {
        emit<MSG::ItemTransfer>( gameId, MSG::ItemTransfer {
            .itemId   = i->itemId,
            .uniqueId = uniqueId,
            .from     = v.belongTo,
            .to       = player
        });
        v.belongTo = player;
    });
}

////////////////////////////////////////////////////
// ORACLE ACTIONS
////////////////////////////////////////////////////

void NFT_973::channel( uint32_t gameId, name contract, name action )
{
    name owner = checkGame<_C<name>, "gameOwner"_m>( gameId );
    require_auth( owner );

    if ( auto i = m_tbChannel.find(gameId); i != m_tbChannel.end() )
    {
        m_tbChannel.modify( i, owner, [&] (auto &v) {
            v.contract = contract;
            v.action   = action;
        });
    }
    else
    {
        m_tbChannel.emplace( owner, [&] (auto &v) {
            v.gameId   = gameId;
            v.contract = contract;
            v.action   = action;
        });
    }

    emit<MSG::ItemTransfer>( gameId, MSG::ItemTransfer {
        .itemId   = 0,
        .uniqueId = 0,
        .from     = get_self(),
        .to       = contract
    });
}
