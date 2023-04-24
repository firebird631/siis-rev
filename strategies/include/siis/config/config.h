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
 * @todo maybe add quote-to-base mode
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
 *
 * Contain any elements of configuration.
 *
 * Different between profile and strategy configuration file :
 * - Strategy file based configuration is light and new format for SiiS.
 * - Profile file based configureation is original SiiS format coming from Python version. It is kept for many reasons.
 *  - First reason for compatibility,
 *  - Second reason because SiiS Python can call strategy backtests/training/learning directly in way to improve
 *  performance.
 *
 *  Either load a strategy configuration, either a profile configuration file.
 *  After that it is allowed to override somes paramters using a learning configuration file.
 *
 *  Supervisor for now are not implemented and might change.
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
    void loadProfileSpec(const o3d::String filename);
    void loadLearningSpec(const o3d::String filename);
    void loadSupervisorSpec(const o3d::String filename);

    HandlerType getHandlerType() const { return m_handlerType; }
    o3d::Bool isPaperMode() const { return m_paperMode; }
    o3d::Bool isNoInteractive() const { return m_noInteractive; }

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
    // cache
    //

    const o3d::String& getCacheType() const { return m_cacheType; }
    const o3d::String& getCacheHost() const { return m_cacheHost; }
    o3d::UInt32 getCachePort() const { return m_cachePort; }
    const o3d::String& getCacheName() const { return m_cacheName; }
    const o3d::String& getCacheUser() const { return m_cacheUser; }
    const o3d::String& getCachePwd() const { return m_cachePwd; }

    //
    // content paths
    //

    const o3d::Dir& getConfigPath() const { return m_configPath; }
    const o3d::Dir& getStrategiesPath() const { return m_strategiesPath; }
    const o3d::Dir& getProfilesPath() const { return m_profilesPath; }
    const o3d::Dir& getMarketsPath() const { return m_marketsPath; }
    const o3d::Dir& getLogsPath() const { return m_logsPath; }
    const o3d::Dir& getReportsPath() const { return m_reportsPath; }
    const o3d::Dir& getLearningPath() const { return m_learningPath; }

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
     */
    const o3d::String& getStrategyFilename() const { return m_strategyFilename; }

    /**
     * @brief getProfileFilename Filename defined by loadProfileSpec.
     */
    const o3d::String& getProfileFilename() const { return m_profileFilename; }

    /**
     * @brief getLearningFilename Filename defined by loadLearningSpec.
     */
    const o3d::String& getLearningFilename() const { return m_learningFilename; }

    /**
     * @brief getAuthor Profile/strategy author nmae.
     */
    const o3d::String& getAuthor() const { return m_author; }

    /**
     * @brief getCreated Profile/strategy creation date and time.
     */
    const o3d::DateTime& getCreated() const { return m_created; }

    /**
     * @brief getModified Profile/strategy revision date and time.
     */
    const o3d::DateTime& getModified() const { return m_modified; }

    /**
     * @brief getRevision Profile/strategy revision number.
     */
    const o3d::Int32 getRevision() const { return m_revision; }

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
    o3d::Dir m_profilesPath;
    o3d::Dir m_supervisorsPath;
    o3d::Dir m_marketsPath;
    o3d::Dir m_logsPath;
    o3d::Dir m_reportsPath;
    o3d::Dir m_learningPath;

    HandlerType m_handlerType;
    o3d::Bool m_paperMode;
    o3d::Bool m_noInteractive;

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

    o3d::String m_cacheType;
    o3d::String m_cacheName;
    o3d::String m_cacheHost;
    o3d::UInt32 m_cachePort;
    o3d::String m_cacheUser;
    o3d::String m_cachePwd;

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

    o3d::String m_profileFilename;
    o3d::String m_strategyFilename;
    o3d::String m_supervisorFilename;
    o3d::String m_learningFilename;

    o3d::String m_author;
    o3d::DateTime m_created;
    o3d::DateTime m_modified;
    o3d::Int32 m_revision;
};

} // namespace siis

#endif // SIIS_CONFIG_H
