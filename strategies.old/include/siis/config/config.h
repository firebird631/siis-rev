/**
 * @brief SiiS strategy config.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_CONFIG_H
#define SIIS_CONFIG_H

#include <o3d/core/dir.h>
#include <o3d/core/stringmap.h>
#include <o3d/core/stringlist.h>
#include <o3d/core/commandline.h>
#include <o3d/core/stringlist.h>

namespace siis {

class StrategyConfig;

/**
 * @brief Per market specific config.
 */
class MarketConfig
{
public:

    enum TradeMode
    {
        TRADE_FIXED_QUANTITY = 0,
        TRADE_MIN_MAX_QUANTITY = 1,
        TRADE_ACCOUNT_PERCENT = 2,
        TRADE_MIN_MAX_ACCOUNT_PERCENT = 3
    };

    o3d::String marketId;

    o3d::String mappedBrokerId;
    o3d::String mappedMarketId;

    o3d::T_StringList multipleMarketsId;

    TradeMode tradeMode;
    o3d::Double tradeQuantity[2];  //!< [min,max] or at index 0 fixed quantity/percent
};

/**
 * @brief Worker for the pool executor.
 * @author Frederic Scherma
 * @date 2019-03-05
 */
class Config
{
public:

    enum HandlerType {
        HANDLER_UNDEFINED = 0,
        HANDLER_LIVE = 1,
        HANDLER_BACKTEST,
        HANDLER_LEARN,
        HANDLER_OPTIMIZE
    };

    Config();
    virtual ~Config();

    void initPaths(const o3d::Dir &basePath);
    void loadCmdLine(const o3d::CommandLine *cmdLine);

    void loadCommon();

    void loadStrategySpec(const o3d::String filename);
    void loadSupervisorSpec(const o3d::String filename);

    HandlerType getHandlerType() const { return m_handlerType; }
    o3d::Bool isPaperMode() const { return m_paperMode; }

    //
    // connector
    //

    const o3d::String& getBrokerId() const { return m_brokerId; }
    const o3d::String& getConnectorHost() const { return m_connectorHost; }
    o3d::UInt32 getConnectorPort() const { return m_connectorPort; }
    const o3d::String& getConnectorKey() { return m_connectorKey; }

    //
    // database
    //

    const o3d::String& getDBType() const { return m_dbType; }
    const o3d::String& getDBHost() const { return m_dbHost; }
    o3d::UInt32 getDBPort() const { return m_dbPort; }
    const o3d::String& getDBName() const { return m_dbName; }
    const o3d::String& getDBUser() const { return m_dbUser; }
    const o3d::String& getDBPwd() const { return m_dbPwd; }

    //
    // content paths
    //

    const o3d::Dir& getConfigPath() const { return m_configPath; }
    const o3d::Dir& getStrategiesPath() const { return m_strategiesPath; }
    const o3d::Dir& getMarketsPath() const { return m_marketsPath; }
    const o3d::Dir& getLogsPath() const { return m_logsPath; }
    const o3d::Dir& getReportsPath() const { return m_reportsPath; }

    //
    // strategy and workers
    //

    o3d::Int32 getNumWorkers() const { return m_numWorkers; }

    const o3d::String& getStrategy() const { return m_strategy; }
    const o3d::String& getStrategyIdentifier() const { return m_strategyIdentifier; }

    const o3d::StringMap<o3d::String> getCollection() const { return m_collectionClassPath; }
    const o3d::StringMap<o3d::String> getIndicators() const { return m_indicatorClassPath; }

    /**
     * @brief getStrategyFilename Filename defined by loadStrategySpec.
     * @return
     */
    const o3d::String& getStrategyFilename() const { return m_strategyFilename; }

    //
    // supervisor for machine learning
    //

    const o3d::String& getSupervisor() const { return m_supervisor; }
    const o3d::String& getSupervisorIdentifier() const { return m_supervisorIdentifier; }

    /**
     * @brief getSupervisorFilename Filename defined by loadSupervisorSpec.
     * @return
     */
    const o3d::String& getSupervisorFilename() const { return m_supervisorFilename; }

    /**
     * @brief fromTs From datetime converted to UTC timestamp.
     */
    o3d::Double getFromTs() const { return m_fromTs; }

    /**
     * @brief toTs To datetime converted to UTC timestamp.
     */
    o3d::Double getToTs() const { return m_toTs; }

    /**
     * @brief timestep Timestep (compatible with TF_ prefixed timeframe).
     */
    o3d::Double getTimestep() const { return m_timestep; }

    /**
     * @brief markets List of configured markets.
     * Markets are either a complet market-id, or prefixed by a negation ! to exlude it,
     * or prefixed by a wildchar * to allow any markets - knew from the broker - ending with the pattern.
     * For example : {"BTCUSDT","*BTC","!DOGEBTC"}.
     */
    const o3d::T_StringList getMarkets() const { return m_markets; }

    /**
     * @brief marketsConfig Configured market from the configuration file.
     */
    const std::list<MarketConfig*> getConfiguredMarkets() const { return m_configuredMarkets; }

private:

    o3d::Dir m_configPath;
    o3d::Dir m_strategiesPath;
    o3d::Dir m_supervisorsPath;
    o3d::Dir m_marketsPath;
    o3d::Dir m_logsPath;
    o3d::Dir m_reportsPath;

    HandlerType m_handlerType;
    o3d::Bool m_paperMode;

    o3d::String m_brokerId;
    o3d::String m_connectorHost;
    o3d::UInt32 m_connectorPort;
    o3d::String m_connectorKey;

    o3d::String m_dbType;
    o3d::String m_dbName;
    o3d::String m_dbHost;
    o3d::UInt32 m_dbPort;
    o3d::String m_dbUser;
    o3d::String m_dbPwd;

    o3d::Int32 m_numWorkers;

    o3d::String m_strategy;
    o3d::String m_strategyIdentifier;

    o3d::String m_supervisor;
    o3d::String m_supervisorIdentifier;

    o3d::StringMap<o3d::String> m_collectionClassPath;
    o3d::StringMap<o3d::String> m_indicatorClassPath;

    o3d::Double m_fromTs;
    o3d::Double m_toTs;
    o3d::Double m_timestep;

    std::list<MarketConfig*> m_configuredMarkets;
    o3d::T_StringList m_markets;

    o3d::String m_strategyFilename;
    o3d::String m_supervisorFilename;
};

} // namespace siis

#endif // SIIS_CONFIG_H
