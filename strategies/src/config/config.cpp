/**
 * @brief SiiS strategy config.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/config/config.h"
#include "siis/config/strategyconfig.h"
#include "siis/market.h"
#include "siis/statistics/statistics.h"

#include <o3d/core/filemanager.h>
#include <o3d/core/file.h>
#include <o3d/core/debug.h>
#include <o3d/core/datetime.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

Config::Config() :
    m_configPath("."),
    m_strategiesPath("."),
    m_profilesPath("."),
    m_marketsPath("."),
    m_logsPath("."),
    m_reportsPath("."),
    m_learningPath("."),
    m_handlerType(HANDLER_LIVE),
    m_paperMode(false),
    m_noInteractive(false),
    m_dbType("postgresql"),
    m_dbName("siis"),
    m_dbHost("127.0.0.1"),
    m_dbPort(5432),
    m_dbUser("siis"),
    m_dbPwd("siis"),
    m_cacheType("redis"),
    m_cacheName("siis"),
    m_cacheHost("127.0.0.1"),
    m_cachePort(6379),
    m_cacheUser("siis"),
    m_cachePwd("siis"),
    m_numWorkers(-1),
    m_fromTs(0),
    m_toTs(0),
    m_timestep(1),
    m_author(""),
    m_created(),
    m_modified(),
    m_revision(1),
    m_initialBalance(0.0),
    m_initialCurrency("USD")
{

}

Config::~Config()
{
    for (MarketConfig *mc : m_configuredMarkets) {
        o3d::deletePtr(mc);
    }
}

void Config::initPaths(const o3d::Dir &basePath)
{
    m_configPath = basePath;

    O3D_ASSERT(m_configPath.exists());
    m_configPath.cd("config");

    if (m_configPath.check("connectors") != 0) {
        m_configPath.makeDir("connectors");
    }

    m_strategiesPath = m_configPath;
    if (m_strategiesPath.check("strategies") != 0) {
        m_strategiesPath.makeDir("strategies");
    }
    m_strategiesPath.cd("strategies");

    m_profilesPath = m_configPath;
    if (m_profilesPath.check("profiles") != 0) {
        m_profilesPath.makeDir("profiles");
    }
    m_profilesPath.cd("profiles");

    m_supervisorsPath = m_configPath;
    if (m_supervisorsPath.check("supervisors") != 0) {
        m_supervisorsPath.makeDir("supervisors");
    }
    m_supervisorsPath.cd("supervisors");

    m_marketsPath = basePath;
    if (m_marketsPath.check("markets") != 0) {
        m_marketsPath.makeDir("markets");
    }
    m_marketsPath.cd("markets");

    m_logsPath = basePath;
    if (m_logsPath.check("log") != 0) {
        m_logsPath.makeDir("log");
    }
    m_logsPath.cd("log");

    m_reportsPath = basePath;
    if (m_reportsPath.check("reports") != 0) {
        m_reportsPath.makeDir("reports");
    }
    m_reportsPath.cd("reports");

    m_learningPath = basePath;
    if (m_learningPath.check("learning") != 0) {
        m_learningPath.makeDir("learning");
    }
    m_learningPath.cd("learning");
}

void Config::loadCmdLine(const o3d::CommandLine *cmdLine)
{
    O3D_ASSERT(cmdLine != nullptr);

    // parse cmdLine extra arguments
    if (cmdLine) {
        m_markets = cmdLine->getOptionValues('m');

        o3d::String fromDt = cmdLine->getOptionValue('f');
        o3d::String toDt = cmdLine->getOptionValue('t');
        o3d::String timestep = cmdLine->getOptionValue('i');

        if (timestep.isValid()) {
            m_timestep = timestep.toDouble();
        }

        if (fromDt.isValid()) {
            o3d::DateTime dt;
            if (!dt.buildFromString(fromDt, "%Y-%m-%dT%H:%M:%S")) {
                O3D_ERROR(o3d::E_InvalidFormat("From date time"));
            }

            m_fromTs = dt.toDoubleTimestamp(true);
        }

        if (toDt.isValid()) {
            o3d::DateTime dt;
            if (!dt.buildFromString(toDt, "%Y-%m-%dT%H:%M:%S")) {
                O3D_ERROR(o3d::E_InvalidFormat("To date time"));
            }

            m_toTs = dt.toDoubleTimestamp(true);
        } else {
            m_toTs = static_cast<o3d::Double>(o3d::System::getMsTime()) / 1000.0;
        }

        if (cmdLine->getSwitch('b') && cmdLine->getSwitch('p')) {
            O3D_ERROR(o3d::E_InvalidParameter("backtest switch is incompatible with paper"));
        }

        if (cmdLine->getSwitch('b') && cmdLine->getSwitch('l')) {
            O3D_ERROR(o3d::E_InvalidParameter("backtest switch is incompatible with learn"));
        }

        if (cmdLine->getSwitch('b') && cmdLine->getSwitch('o')) {
            O3D_ERROR(o3d::E_InvalidParameter("backtest switch is incompatible with optimize"));
        }

        if (cmdLine->getSwitch('o') && cmdLine->getSwitch('l')) {
            O3D_ERROR(o3d::E_InvalidParameter("optimize and learn switches are mutually exclusives"));
        }

        if (cmdLine->getSwitch('b')) {
            m_handlerType = HANDLER_BACKTEST;
        } else if (cmdLine->getSwitch('L')) {
            m_handlerType = HANDLER_LEARN;
        } else if (cmdLine->getSwitch('o')) {
            m_handlerType = HANDLER_OPTIMIZE;
        } else if (cmdLine->getSwitch('l')) {
            m_handlerType = HANDLER_LIVE;
        } else {
            m_handlerType = HANDLER_UNDEFINED;
        }

        if (cmdLine->getSwitch('p')) {
            m_paperMode = true;
        } else {
            m_paperMode = false;
        }

        if (cmdLine->getSwitch('n')) {
            m_noInteractive = true;
        } else {
            m_noInteractive= false;
        }
    }
}

void Config::loadCommon()
{
    o3d::File lfile(m_configPath.getFullPathName(), "strategy.json");
    if (!lfile.exists()) {
        O3D_ERROR(o3d::E_InvalidParameter("strategy.json configuration file not found"));
    }

    JsonParser parser;
    if (parser.parse(m_configPath, "strategy.json")) {
        // global
        Json::Value global = parser.root().get("global", Json::Value());
        m_numWorkers = global.get("workers", -1).asInt();

        // database
        Json::Value database = parser.root().get("database", Json::Value());
        m_dbType = database.get("type", "postgresql" /*mysql*/).asString().c_str();
        m_dbPort = static_cast<o3d::UInt32>(database.get("port", 5432 /*3306*/).asInt());

        m_dbHost = database.get("host", "127.0.0.1").asString().c_str();
        m_dbName = database.get("name", "siis").asString().c_str();
        m_dbUser = database.get("user", "siis").asString().c_str();
        m_dbPwd = database.get("pwd", "siis").asString().c_str();

        // cache
        Json::Value cache = parser.root().get("cache", Json::Value());
        m_cacheType = cache.get("type", "redis").asString().c_str();
        m_cachePort = static_cast<o3d::UInt32>(cache.get("port", 6379).asInt());

        m_cacheHost = cache.get("host", "127.0.0.1").asString().c_str();
        m_cacheName = cache.get("name", "siis").asString().c_str();
        m_cacheUser = cache.get("user", "siis").asString().c_str();
        m_cachePwd = cache.get("pwd", "siis").asString().c_str();

        // indicators
        Json::Value indicators = parser.root().get("indicators", Json::Value());
        // ...
    }
}

o3d::Int32 marketTradeTypeFromStr(const o3d::String &type)
{
    if (type == "spot" || type == "asset" || type == "buysell") {
        return Market::TRADE_BUY_SELL;
    } else if (type == "margin") {
        return Market::TRADE_MARGIN;
    } else if (type == "ind-margin") {
        return Market::TRADE_IND_MARGIN;
    } else if (type == "position") {
        return Market::TRADE_POSITION;
    } else {
        O3D_ERROR(o3d::E_InvalidParameter(o3d::String("{0} is not a valide mode").arg(type)));
    }
}

void Config::loadStrategySpec(const o3d::String filename)
{
    o3d::File lfile(m_strategiesPath.getFullPathName(), filename);
    if (!lfile.exists()) {
        O3D_ERROR(o3d::E_InvalidParameter(o3d::String("{0} strategy configuration file not found").arg(filename)));
    }

    MarketConfig::TradeMode defaultTradeMode = MarketConfig::TRADE_FIXED_QUANTITY;
    o3d::Double defaultTradeQuantity[] = {1.0, 1.0};

    try {
        JsonParser parser;
        if (parser.parse(m_strategiesPath, filename)) {
            // global
            Json::Value global = parser.root().get("global", Json::Value());

            // virtual account settings
            m_initialCurrency = global.get("initial-currency", "USD").asCString();
            m_initialBalance = global.get("initial-balance", 1000.0).asDouble();

            // overrides
            m_numWorkers = global.get("workers", m_numWorkers).asInt();

            // specific
            m_strategy = global.get("strategy", "").asString().c_str();
            m_strategyIdentifier = global.get("identifier", "").asString().c_str();

            // connector
            Json::Value connector = parser.root().get("connector", Json::Value());

            m_brokerId = connector.get("broker-id", "").asString().c_str();
            m_connectorHost = connector.get("host", "127.0.0.1").asString().c_str();
            m_connectorPort = static_cast<o3d::UInt32>(connector.get("port", 6401).asInt());
            m_connectorKey = connector.get("key", "").asString().c_str();

            if (m_brokerId.isEmpty()) {
                O3D_ERROR(o3d::E_InvalidParameter("Undefined connector brokerId"));
            }

            if (m_connectorHost.isEmpty()) {
                O3D_ERROR(o3d::E_InvalidParameter("Undefined connector host"));
            }

            // default market conf
            if (global.isMember("trade-mode")) {
                o3d::String tradeMode = global.get("trade-mode", "fixed-quantity").asString().c_str();
                if (tradeMode == "fixed-quantity") {
                    defaultTradeMode = MarketConfig::TRADE_FIXED_QUANTITY;
                } else if (tradeMode == "min-max-quantity") {
                    defaultTradeMode = MarketConfig::TRADE_MIN_MAX_QUANTITY;
                } else if (tradeMode == "account-percent") {
                    defaultTradeMode = MarketConfig::TRADE_ACCOUNT_PERCENT;
                } else if (tradeMode == "min-max-account-percent") {
                    defaultTradeMode = MarketConfig::TRADE_MIN_MAX_ACCOUNT_PERCENT;
                } else {
                    o3d::String msg = o3d::String("Unsupported default market tradeMode value {0}").arg(defaultTradeMode);
                    O3D_ERROR(o3d::E_InvalidFormat(msg));
                }
            }

            if (defaultTradeMode == MarketConfig::TRADE_FIXED_QUANTITY || defaultTradeMode == MarketConfig::TRADE_ACCOUNT_PERCENT) {
                if (!global.isMember("quantity")) {
                    o3d::String msg = o3d::String("Missing default quantity");
                    O3D_ERROR(o3d::E_InvalidFormat(msg));
                }

                defaultTradeQuantity[0] = defaultTradeQuantity[1] = global.get("quantity", 0.0).asDouble();
            }

            if (defaultTradeMode == MarketConfig::TRADE_MIN_MAX_QUANTITY || defaultTradeMode == MarketConfig::TRADE_MIN_MAX_ACCOUNT_PERCENT) {
                if (!global.isMember("min-quantity") || !global.isMember("max-quantity")) {
                    o3d::String msg = o3d::String("Missing default min-quantity or max-quantity key");
                    O3D_ERROR(o3d::E_InvalidFormat(msg));
                }

                defaultTradeQuantity[0] = global.get("min-quantity", 0.0).asDouble();
                defaultTradeQuantity[1] = global.get("max-quantity", 0.0).asDouble();
            }

            // markets conf
            Json::Value markets = parser.root().get("markets", Json::Value());
            for (auto it = markets.begin(); it!= markets.end(); ++it) {
                Json::Value market = *it;

                MarketConfig *mc = new MarketConfig;
                mc->marketId = it.name().c_str();

                if (market.isMember("mapped-broker-id") && market.isMember("mapped-market-id")) {
                    mc->mappedBrokerId = market.get("mapped-broker-id", "").asString().c_str();
                    mc->mappedMarketId = market.get("mapped-market-id", "").asString().c_str();
                }

                if (market.isMember("multiple-markets-id")) {
                    // for multiple market list, case of backtesting on a future contract, have history of a previous market
                    Json::Value multipleMarketsId = market.get("multiple-markets-id", Json::Value());
                    for (auto it = multipleMarketsId.begin(); it != multipleMarketsId.end(); ++it) {
                        mc->multipleMarketsId.push_back((*it).asCString());
                    }
                }

                if (market.isMember("trade-mode")) {
                    o3d::String tradeMode = market.get("trade-mode", "fixed-quantity").asString().c_str();
                    if (tradeMode == "fixed-quantity") {
                        mc->tradeMode = MarketConfig::TRADE_FIXED_QUANTITY;
                    } else if (tradeMode == "min-max-quantity") {
                        mc->tradeMode = MarketConfig::TRADE_MIN_MAX_QUANTITY;
                    } else if (tradeMode == "account-percent") {
                        mc->tradeMode = MarketConfig::TRADE_ACCOUNT_PERCENT;
                    } else if (tradeMode == "min-max-account-percent") {
                        mc->tradeMode = MarketConfig::TRADE_MIN_MAX_ACCOUNT_PERCENT;
                    } else {
                        o3d::String msg = o3d::String("Unsupported market tradeMode value {0} for {1}").arg(tradeMode).arg(mc->marketId);
                        o3d::deletePtr(mc);
                        O3D_ERROR(o3d::E_InvalidFormat(msg));
                    }
                } else {
                    mc->tradeMode = defaultTradeMode;
                }

                if (market.isMember("trade-mode")) {
                    if (mc->tradeMode == MarketConfig::TRADE_FIXED_QUANTITY || mc->tradeMode == MarketConfig::TRADE_ACCOUNT_PERCENT) {
                        if (!market.isMember("quantity")) {
                            o3d::String msg = o3d::String("Missing quantity key for {0}").arg(mc->marketId);
                            o3d::deletePtr(mc);
                            O3D_ERROR(o3d::E_InvalidFormat(msg));
                        }

                        mc->tradeQuantity[0] = mc->tradeQuantity[1] = market.get("quantity", 0.0).asDouble();
                    }
                    else if (mc->tradeMode == MarketConfig::TRADE_MIN_MAX_QUANTITY || mc->tradeMode == MarketConfig::TRADE_MIN_MAX_ACCOUNT_PERCENT)
                    {
                        if (!market.isMember("min-quantity") || !market.isMember("max-quantity")) {
                            o3d::String msg = o3d::String("Missing min-quantity or max-quantity key for {0}").arg(mc->marketId);
                            o3d::deletePtr(mc);
                            O3D_ERROR(o3d::E_InvalidFormat(msg));
                        }

                        mc->tradeQuantity[0] = market.get("min-quantity", 0.0).asDouble();
                        mc->tradeQuantity[1] = market.get("max-quantity", 0.0).asDouble();
                    }
                } else {
                    mc->tradeQuantity[0] = defaultTradeQuantity[0];
                    mc->tradeQuantity[1] = defaultTradeQuantity[1];
                }

                // initial market detail but will be overrided if found in DB
                if (market.isMember("trade-type")) {
                    o3d::String tradeType = market.get("trade-type", "").asString().c_str();
                    mc->marketTradeType = marketTradeTypeFromStr(tradeType);
                }

                // ...

                m_configuredMarkets.push_back(mc);
            }
        }

        m_strategyFilename = filename;
    }
    catch (Json::LogicError &e) {
        O3D_ERROR(o3d::E_InvalidParameter("Invalid JSON format for strategy " + filename));
    }
}

void Config::loadProfileSpec(const o3d::String filename)
{
    o3d::File lfile(m_profilesPath.getFullPathName(), filename);
    if (!lfile.exists()) {
        throw o3d::E_InvalidParameter(o3d::String("{0} profile configuration file not found").arg(filename));
    }

    MarketConfig::TradeMode defaultTradeMode = MarketConfig::TRADE_FIXED_QUANTITY;
    o3d::Double defaultTradeQuantity[] = {1.0, 1.0};

    try {
        JsonParser parser;
        if (parser.parse(m_profilesPath, filename)) {
            // root
            m_author = parser.root().get("author", "").asString().c_str();

            o3d::String created = parser.root().get("created", "").asString().c_str();
            o3d::String modified = parser.root().get("modified", "").asString().c_str();

            m_created.buildFromString(created, o3d::String("%Y-%m-%dT%H:%M:%S"));
            m_modified.buildFromString(modified, o3d::String("%Y-%m-%dT%H:%M:%S"));

            m_revision = parser.root().get("revision", 1).asInt();

            // overrides
            m_numWorkers = parser.root().get("workers", m_numWorkers).asInt();

            // trader/connector
            Json::Value trader = parser.root().get("trader", Json::Value());

            // virtual account settings
            if (trader.isMember("paper-mode")) {
                Json::Value paperMode = trader.get("paper-mode", Json::Value());

                o3d::String type = paperMode.get("type", "margin").asCString();
                if (type == "asset" && paperMode.isMember("asset")) {
                    Json::Value assets = paperMode.get("assets", Json::Value());

                    for (auto it = assets.begin(); it!= assets.end(); ++it) {
                        m_initialCurrency = it->get("base", "USDT").asCString();
                        m_initialBalance = it->get("base", 1000.0).asDouble();

                        // only keep the first one
                        break;
                    }
                } else if (type == "margin") {
                    m_initialCurrency = paperMode.get("currency", "USD").asCString();
                    m_initialBalance = paperMode.get("initial", 1000.0).asDouble();
                }
            }

            m_brokerId = trader.get("name", "").asString().c_str();
            // m_connectorHost = connector.get("host", "127.0.0.1").asString().c_str();
            // m_connectorPort = static_cast<o3d::UInt32>(connector.get("port", 6401).asInt());
            // m_connectorKey = connector.get("key", "").asString().c_str();

            if (m_brokerId.isEmpty()) {
                O3D_ERROR(o3d::E_InvalidParameter("Undefined connector brokerId"));
            }

            // only for live mode @todo
            //        if (m_connectorHost.isEmpty()) {
            //            O3D_ERROR(o3d::E_InvalidParameter("Undefined connector host"));
            //        }

            //        // default market conf
            //        if (global.isMember("trade-mode")) {
            //            o3d::String tradeMode = global.get("trade-mode", "fixed-quantity").asString().c_str();
            //            if (tradeMode == "fixed-quantity") {
            //                defaultTradeMode = MarketConfig::TRADE_FIXED_QUANTITY;
            //            } else if (tradeMode == "min-max-quantity") {
            //                defaultTradeMode = MarketConfig::TRADE_MIN_MAX_QUANTITY;
            //            } else if (tradeMode == "account-percent") {
            //                defaultTradeMode = MarketConfig::TRADE_ACCOUNT_PERCENT;
            //            } else if (tradeMode == "min-max-account-percent") {
            //                defaultTradeMode = MarketConfig::TRADE_MIN_MAX_ACCOUNT_PERCENT;
            //            } else {
            //                o3d::String msg = o3d::String("Unsupported default market tradeMode value {0}").arg(defaultTradeMode);
            //                O3D_ERROR(o3d::E_InvalidFormat(msg));
            //            }
            //        }

            //        if (defaultTradeMode == MarketConfig::TRADE_FIXED_QUANTITY || defaultTradeMode == MarketConfig::TRADE_ACCOUNT_PERCENT) {
            //            if (!global.isMember("quantity")) {
            //                o3d::String msg = o3d::String("Missing default quantity");
            //                O3D_ERROR(o3d::E_InvalidFormat(msg));
            //            }

            //            defaultTradeQuantity[0] = defaultTradeQuantity[1] = global.get("quantity", 0.0).asDouble();
            //        }

            //        if (defaultTradeMode == MarketConfig::TRADE_MIN_MAX_QUANTITY || defaultTradeMode == MarketConfig::TRADE_MIN_MAX_ACCOUNT_PERCENT) {
            //            if (!global.isMember("min-quantity") || !global.isMember("max-quantity")) {
            //                o3d::String msg = o3d::String("Missing default min-quantity or max-quantity key");
            //                O3D_ERROR(o3d::E_InvalidFormat(msg));
            //            }

            //            defaultTradeQuantity[0] = global.get("min-quantity", 0.0).asDouble();
            //            defaultTradeQuantity[1] = global.get("max-quantity", 0.0).asDouble();
            //        }

            // markets conf
            Json::Value instruments = trader.get("instruments", Json::Value());
            Json::Value symbols = trader.get("symbols", Json::Value());

            // @todo if instrument starting with * :
            // defaultTradeMode
            // defaultTradeQuantity[0] = defaultTradeQuantity[1] = instrument.get("quantity", 0.0).asDouble();

            for (auto it = symbols.begin(); it!= symbols.end(); ++it) {
                // @todo support instrument key whichs start with * and then market-id can contains {0} placeholder
                MarketConfig *mc = new MarketConfig;
                mc->marketId = it->asString().c_str();

                Json::Value instrument = instruments.get(mc->marketId.getData(), Json::Value());
                if (!instrument.empty()) {
                    o3d::String marketId = instrument.get("market-id", "").asString().c_str();
                    mc->tradeQuantity[0] = mc->tradeQuantity[1] = instrument.get("size", 0.0).asDouble();

                    o3d::String sizeMode = instrument.get("size-mode", "base").asString().c_str();
                    if (sizeMode == "quote-to-base") {
                        // @todo
                    } else {

                    }

                    O3D_MESSAGE(o3d::String("Find market {0} min-trade-quantity={1}").arg(mc->marketId).arg(mc->tradeQuantity[0]));

                    mc->tradeMode = MarketConfig::TRADE_FIXED_QUANTITY;

                    // initial market detail but will be overrided if found in DB
                    if (instrument.isMember("trade-type")) {
                        o3d::String tradeType = instrument.get("trade-type", "").asString().c_str();
                        mc->marketTradeType = marketTradeTypeFromStr(tradeType);
                    }

                    // ...
                } else {
                    // must have an instrument with a wildchar and market-id have a {0} placeholder
                    // @todo
                }

                m_configuredMarkets.push_back(mc);
            }

            // watchers/connectors
            Json::Value watchers = parser.root().get("watchers", Json::Value());
            for (auto it = watchers.begin(); it!= watchers.end(); ++it) {
                o3d::String name = it.name().c_str();

                Json::Value symbols = it->get("symbols", Json::Value());
                // @todo
            }

            // strategy
            Json::Value strategy = parser.root().get("strategy", Json::Value());

            m_strategy = strategy.get("name", "").asString().c_str();
            m_strategyIdentifier = strategy.get("id", "").asString().c_str();
        }

        m_profileFilename = filename;
    }
    catch (Json::LogicError &e) {
        O3D_ERROR(o3d::E_InvalidParameter("Invalid JSON format for profile " + filename));
    }
}

void Config::loadLearningSpec(const o3d::String filename)
{
    o3d::File lfile(m_learningPath.getFullPathName(), filename);
    if (!lfile.exists()) {
        throw o3d::E_InvalidParameter(o3d::String("{0} learning configuration file not found").arg(filename));
    }

    try {
        JsonParser parser;
        if (parser.parse(m_learningPath, filename)) {
            // root

            // trader/connector

            // watcher/connectors

            // strategy
            // @todo and overrides
        }

        m_learningFilename = filename;
    }
    catch (Json::LogicError &e) {
        O3D_ERROR(o3d::E_InvalidParameter("Invalid JSON format for learning " + filename));
    }
}

void Config::loadSupervisorSpec(const o3d::String filename)
{
    o3d::File lfile(m_supervisorsPath.getFullPathName(), filename);
    if (!lfile.exists()) {
        O3D_ERROR(o3d::E_InvalidParameter(o3d::String("{0} supervisor configuration file not found").arg(filename)));
    }

    try {
        JsonParser parser;
        if (parser.parse(m_supervisorsPath, filename)) {
            // global
            Json::Value global = parser.root().get("global", Json::Value());

            // specific
            m_supervisor = global.get("supervisor", "").asString().c_str();
            m_supervisorIdentifier = global.get("identifier", "").asString().c_str();

            // @todo
        }

        m_supervisorFilename = filename;
    }
    catch (Json::LogicError &e) {
        O3D_ERROR(o3d::E_InvalidParameter("Invalid JSON format for supervisor " + filename));
    }
}

void Config::overwriteLearningFile(const GlobalStatistics &global, const AccountStatistics &account) const
{
    if (m_learningFilename.isEmpty()) {
        return;
    }

    o3d::File lfile(m_learningPath.getFullPathName(), m_learningFilename);
    if (!lfile.exists()) {
        O3D_ERROR(o3d::E_InvalidParameter(o3d::String("{0} learning configuration file not found").arg(m_learningFilename)));
    }

    try {
        JsonParser parser;
        if (parser.parse(m_learningPath, m_learningFilename)) {
            // add revision, perf stats
            Json::Value &root = parser.modifyRoot();

            o3d::DateTime today;
            today.setCurrent(true);

            root["revision"] = today.buildString("%Y-%m-%dT%H:%M:%SZ").toAscii().getData();

            Json::Value samples(Json::arrayValue);
            // @todo samples
            for (auto sample : account.samples) {
                Json::Value s(Json::arrayValue);

                s.append(sample.timestamp);
                s.append(sample.equity);
                s.append(sample.profitLoss);
                s.append(sample.drawDown);

                samples.append(s);
            }

            // @todo format equity and pnl according to account currency precision
            root["performance"] = o3d::String::print("%.2f%%", global.performance * 100).toAscii().getData();
            root["max-draw-down"] = o3d::String::print("%.2f%%", global.maxDrawDown * 100).toAscii().getData();
            // root["initial-equity"] = o3d::String::print("%.2f", account.initialEquity).toAscii().getData();
            root["final-equity"] = o3d::String::print("%.2f", account.finalEquity).toAscii().getData();
            root["stats-samples"] = samples;
            root["profit-loss"] = o3d::String::print("%.2f", account.profitLoss).toAscii().getData();

            root["best"] = o3d::String::print("%.2f%%", global.best * 100).toAscii().getData();
            root["worst"] = o3d::String::print("%.2f%%", global.worst * 100).toAscii().getData();
            root["succeed-trades"] = global.succeedTrades;
            root["failed-trades"] = global.failedTrades;
            root["roe-trades"] = global.roeTrades;
            root["total-trades"] = global.totalTrades;
            root["open-trades"] = global.openTrades;
            root["active-trades"] = global.activeTrades;
            root["stop-loss-in-loss"] = global.stopLossInLoss;
            root["take-profit-in-loss"] = global.takeProfitInLoss;
            root["stop-loss-in-gain"] = global.stopLossInGain;
            root["take-profit-in-gain"] = global.takeProfitInGain;

            parser.save(m_learningPath.getFullPathName(), m_learningFilename);
        }
    }
    catch (Json::LogicError &e) {
        O3D_ERROR(o3d::E_InvalidParameter("Invalid JSON format for learning " + m_learningFilename));
    }
}

void Config::deleteLearningFile(const o3d::String &filename)
{
    if (filename.isEmpty()) {
        return;
    }

    o3d::File lfile(m_learningPath.getFullPathName(), filename);
    if (lfile.exists()) {
        m_learningPath.removeFile(filename);
    }
}
