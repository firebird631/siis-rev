/**
 * @brief SiiS strategy config.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/config/config.h"
#include "siis/config/strategyconfig.h"

#include <o3d/core/filemanager.h>
#include <o3d/core/file.h>
#include <o3d/core/debug.h>
#include <o3d/core/datetime.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

Config::Config() :
    m_configPath("."),
    m_marketsPath("."),
    m_logsPath("."),
    m_reportsPath("."),
    m_handlerType(HANDLER_LIVE),
    m_paperMode(false),
    m_dbType("mysql"),
    m_dbName("siis"),
    m_dbHost("127.0.0.1"),
    m_dbPort(5132),
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
    m_timestep(1)
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

            m_fromTs = dt.toDoubleTimestamp();
        }

        if (toDt.isValid()) {
            o3d::DateTime dt;
            if (!dt.buildFromString(toDt, "%Y-%m-%dT%H:%M:%S")) {
                O3D_ERROR(o3d::E_InvalidFormat("To date time"));
            }

            m_toTs = dt.toDoubleTimestamp();
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
    }
}

void Config::loadCommon()
{
    o3d::File lfile(m_configPath.getFullPathName(), "strategy.json");
    if (!lfile.exists()) {
        throw o3d::E_InvalidParameter("strategy.json configuration file not found");
    }

    JsonParser parser;
    if (parser.parse(m_configPath, "strategy.json")) {
        // global
        Json::Value global = parser.root().get("global", Json::Value());
        m_numWorkers = global.get("workers", -1).asInt();

        // database
        Json::Value database = parser.root().get("database", Json::Value());
        m_dbType = database.get("type", "mysql").asString().c_str();
        m_dbPort = static_cast<o3d::UInt32>(database.get("port", 3306 /*5432*/).asInt());

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

void Config::loadStrategySpec(const o3d::String filename)
{
    o3d::File lfile(m_strategiesPath.getFullPathName(), filename);
    if (!lfile.exists()) {
        throw o3d::E_InvalidParameter(o3d::String("{0} strategy configuration file not found").arg(filename));
    }

    MarketConfig::TradeMode defaultTradeMode = MarketConfig::TRADE_FIXED_QUANTITY;
    o3d::Double defaultTradeQuantity[] = {1.0, 1.0};

    JsonParser parser;
    if (parser.parse(m_strategiesPath, filename)) {
        // global
        Json::Value global = parser.root().get("global", Json::Value());

        // overrides
        m_numWorkers = global.get("workers", m_numWorkers).asInt();

        // specific
        m_strategy = global.get("strategy", "").asString().c_str();
        m_strategyIdentifier = global.get("identifier", "").asString().c_str();

        // connector
        Json::Value connector = parser.root().get("connector", Json::Value());

        m_brokerId = connector.get("brokerId", "").asString().c_str();
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
        if (global.isMember("tradeMode")) {
            o3d::String tradeMode = global.get("tradeMode", "fixedQuantity").asString().c_str();
            if (tradeMode == "fixedQuantity") {
                defaultTradeMode = MarketConfig::TRADE_FIXED_QUANTITY;
            } else if (tradeMode == "minMaxQuantity") {
                defaultTradeMode = MarketConfig::TRADE_MIN_MAX_QUANTITY;
            } else if (tradeMode == "accountPercent") {
                defaultTradeMode = MarketConfig::TRADE_ACCOUNT_PERCENT;
            } else if (tradeMode == "minMaxAccountPercent") {
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
            if (!global.isMember("minQuantity") || !global.isMember("maxQuantity")) {
                o3d::String msg = o3d::String("Missing default minQuantity or maxQuantity key");
                O3D_ERROR(o3d::E_InvalidFormat(msg));
            }

            defaultTradeQuantity[0] = global.get("minQuantity", 0.0).asDouble();
            defaultTradeQuantity[1] = global.get("maxQuantity", 0.0).asDouble();
        }

        // markets conf
        Json::Value markets = parser.root().get("markets", Json::Value());
        for (auto it = markets.begin(); it!= markets.end(); ++it) {
            Json::Value market = *it;

            MarketConfig *mc = new MarketConfig;
            mc->marketId = it.name().c_str();

            if (market.isMember("mappedBrokerId") && market.isMember("mappedMarketId")) {
                mc->mappedBrokerId = market.get("mappedBrokerId", "").asString().c_str();
                mc->mappedMarketId = market.get("mappedMarketId", "").asString().c_str();
            }

            if (market.isMember("multipleMarketsId")) {
                // for multiple market list, case of backtesting on a futur contract, have history of a previous market
                Json::Value multipleMarketsId = market.get("multipleMarketsId", Json::Value());
                for (auto it = multipleMarketsId.begin(); it != multipleMarketsId.end(); ++it) {
                    mc->multipleMarketsId.push_back((*it).asCString());
                }
            }

            if (market.isMember("tradeMode")) {
                o3d::String tradeMode = market.get("tradeMode", "fixedQuantity").asString().c_str();
                if (tradeMode == "fixedQuantity") {
                    mc->tradeMode = MarketConfig::TRADE_FIXED_QUANTITY;
                } else if (tradeMode == "minMaxQuantity") {
                    mc->tradeMode = MarketConfig::TRADE_MIN_MAX_QUANTITY;
                } else if (tradeMode == "accountPercent") {
                    mc->tradeMode = MarketConfig::TRADE_ACCOUNT_PERCENT;
                } else if (tradeMode == "minMaxAccountPercent") {
                    mc->tradeMode = MarketConfig::TRADE_MIN_MAX_ACCOUNT_PERCENT;
                } else {
                    o3d::String msg = o3d::String("Unsupported market tradeMode value {0} for {1}").arg(tradeMode).arg(mc->marketId);
                    o3d::deletePtr(mc);
                    O3D_ERROR(o3d::E_InvalidFormat(msg));
                }
            } else {
                mc->tradeMode = defaultTradeMode;
            }

            if (market.isMember("tradeMode")) {
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
                    if (!market.isMember("minQuantity") || !market.isMember("maxQuantity")) {
                        o3d::String msg = o3d::String("Missing minQuantity or maxQuantity key for {0}").arg(mc->marketId);
                        o3d::deletePtr(mc);
                        O3D_ERROR(o3d::E_InvalidFormat(msg));
                    }

                    mc->tradeQuantity[0] = market.get("minQuantity", 0.0).asDouble();
                    mc->tradeQuantity[1] = market.get("maxQuantity", 0.0).asDouble();
                }
            } else {
                mc->tradeQuantity[0] = defaultTradeQuantity[0];
                mc->tradeQuantity[1] = defaultTradeQuantity[1];
            }

           m_configuredMarkets.push_back(mc);
        }
    }

    m_strategyFilename = filename;
}

void Config::loadSupervisorSpec(const o3d::String filename)
{
    o3d::File lfile(m_supervisorsPath.getFullPathName(), filename);
    if (!lfile.exists()) {
        throw o3d::E_InvalidParameter(o3d::String("{0} supervisor configuration file not found").arg(filename));
    }

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
