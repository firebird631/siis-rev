/**
 * @brief SiiS strategy market database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "pgsql.h"
#include "pgsqlmarketdb.h"

using namespace siis;

PgSqlMarketDb::PgSqlMarketDb(siis::PgSql *db) :
    m_db(db)
{
    m_db->db()->registerQuery("get-market-margin-factor-info",
                              R"SQL(SELECT margin_factor FROM market WHERE broker_id = $1 AND market_id = $2)SQL");

    m_db->db()->registerQuery("get-market-info",
                              R"SQL(SELECT symbol,
                                        market_type, unit_type, contract_type,
                                        trade_type, orders,
                                        base, base_display, base_precision,
                                        quote, quote_display, quote_precision,
                                        settlement, settlement_display, settlement_precision,
                                        expiry, timestamp,
                                        lot_size, contract_size, base_exchange_rate,
                                        value_per_pip, one_pip_means, margin_factor,
                                        min_size, max_size, step_size,
                                        min_notional, max_notional, step_notional,
                                        min_price, max_price, step_price,
                                        maker_fee, taker_fee, maker_commission, taker_commission FROM market
                                    WHERE broker_id = $1 AND market_id = $2)SQL");

    // @todo store market data
}

PgSqlMarketDb::~PgSqlMarketDb()
{

}

Database *PgSqlMarketDb::db()
{
    return m_db;
}

const Database *PgSqlMarketDb::db() const
{
    return m_db;
}

o3d::Bool PgSqlMarketDb::fetchMarket(const o3d::String &brokerId, const o3d::String &marketId, Market *out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-market-info");
    if (!query) {
        return false;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());

    query->execute();
    if (query->fetch()) {
        // fetch one row
    }

    if (query->getNumRows() != 1) {
        return false;
    }

    out->setPair(query->getOut("symbol").toCString());
    out->setType(static_cast<Market::Type>(query->getOut("market_type").toInt32()),
                 static_cast<Market::Contract>(query->getOut("contract_type").toInt32()),
                 static_cast<Market::Unit>(query->getOut("unit_type").toInt32()));
    out->setTradeCapacities(query->getOut("trade_type").toInt32());
    out->setOrderCapacities(query->getOut("orders").toInt32());
    out->setBaseInfo(query->getOut("base").toCString(), query->getOut("base_precision").toInt32());
    out->setQuoteInfo(query->getOut("quote").toCString(), query->getOut("quote_precision").toInt32());
    out->setSettlementInfo(query->getOut("settlement").toCString(), query->getOut("settlement_precision").toInt32());
    // don't set base quote & settlement display symbol
    // no have hedging in DB
    // out->setExpiry(query->getOut("expiry").toCString());
    out->setDetails(query->getOut("contract_size").toDouble(),
                    query->getOut("lot_size").toDouble(),
                    query->getOut("value_per_pip").toDouble(),
                    query->getOut("one_pip_means").toDouble(),
                    true);

    out->setState(query->getOut("base_exchange_rate").toDouble(), true);

    if (query->getOut("margin_factor").toCString() == "-") {
        out->setMarginFactor(1.0);
    } else {
        out->setMarginFactor(query->getOut("margin_factor").toDouble());
    }

    out->setPriceFilter(query->getOut("min_price").toDouble(),
                        query->getOut("max_price").toDouble(),
                        query->getOut("step_price").toDouble());
    out->setNotionalFilter(query->getOut("min_notional").toDouble(),
                           query->getOut("max_notional").toDouble(),
                           query->getOut("step_notional").toDouble());
    out->setQtyFilter(query->getOut("min_size").toDouble(),
                      query->getOut("max_size").toDouble(),
                      query->getOut("step_size").toDouble());

    out->setMakerFee(
                query->getOut("maker_fee").toDouble(),
                query->getOut("maker_commission").toDouble());
    out->setTakerFee(
                query->getOut("taker_fee").toDouble(),
                query->getOut("taker_commission").toDouble());

    return true;
}

std::list<Market *> PgSqlMarketDb::fetchMarketList(const o3d::String &brokerId, const o3d::T_StringList &marketsId)
{
    std::list<Market *> results;

    return results;
}
