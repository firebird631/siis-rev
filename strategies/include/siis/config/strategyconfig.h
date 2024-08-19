/**
 * @brief SiiS strategy config with defaults values.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_STRATEGYCONFIG_H
#define SIIS_STRATEGYCONFIG_H

#include "jsonparser.h"
#include "../trade/trade.h"

namespace siis {

class Config;

/**
 * @brief Per indicator specific config.
 */
class SIIS_API IndicatorConfig
{
public:

    IndicatorConfig(Json::Value &data) :
        m_data(data)
    {
    }

    /**
     * @brief data Tree relative to the indicator configuration.
     */
    const Json::Value& data() const { return m_data; }

private:

    Json::Value m_data;
};

/**
 * @brief Per analyser specific config.
 */
class SIIS_API AnalyserConfig
{
public:

    AnalyserConfig(Json::Value &data) :
        m_data(data)
    {
    }

    /**
     * @brief data Tree relative to the analyser configuration.
     */
    const Json::Value& data() const { return m_data; }

    /**
     * @brief indicator sub-tree.
     */
    Json::Value indicators() const { return m_data.get("indicators", Json::Value()); }

    o3d::Bool hasIndicator(const o3d::CString &name) const;

    o3d::Int32 getAnalyserInt(const o3d::CString &analyserName,
                              const o3d::CString &valueName,
                              o3d::Int32 defValue) const;

    o3d::Double getAnalyserDouble(const o3d::CString &analyserName,
                                  const o3d::CString &valueName,
                                  o3d::Double defValue) const;

private:

    Json::Value m_data;
};

/**
 * @brief Context entry specific config.
 */
class SIIS_API EntryExitConfig
{
public:

    EntryExitConfig(const Json::Value &data) :
        m_data(data)
    {
    }

    /**
     * @brief data Tree relative to the context entry configuration.
     */
    const Json::Value& data() const { return m_data; }

private:

    Json::Value m_data;
};

/**
 * @brief Context entry specific config.
 */
class SIIS_API EntryConfig : public EntryExitConfig
{
public:

    EntryConfig(const Json::Value &data) :
        EntryExitConfig(data)
    {
    }
};

/**
 * @brief Context take-profit specific config.
 */
class SIIS_API TakeProfitConfig : public EntryExitConfig
{
public:

    TakeProfitConfig(const Json::Value &data) :
        EntryExitConfig(data)
    {
    }
};

/**
 * @brief Context stop-loss specific config.
 */
class SIIS_API StopLossConfig : public EntryExitConfig
{
public:

    StopLossConfig(const Json::Value &data) :
        EntryExitConfig(data)
    {
    }
};

/**
 * @brief Context breakeven specific config.
 */
class SIIS_API BreakevenConfig : public EntryExitConfig
{
public:

    BreakevenConfig(const Json::Value &data) :
        EntryExitConfig(data)
    {
    }
};

/**
 * @brief Context dynamic stop-loss specific config.
 */
class SIIS_API DynamicStopLossConfig : public EntryExitConfig
{
public:

    DynamicStopLossConfig(const Json::Value &data) :
        EntryExitConfig(data)
    {
    }
};

/**
 * @brief Context dynamic take-profit specific config.
 */
class SIIS_API DynamicTakeProfitConfig : public EntryExitConfig
{
public:

    DynamicTakeProfitConfig(const Json::Value &data) :
        EntryExitConfig(data)
    {
    }
};

/**
 * @brief Per context specific config.
 */
class SIIS_API ContextConfig
{
public:

    ContextConfig(const Json::Value &data) :
        m_data(data)
    {
    }

    /**
     * @brief data Tree relative to the context configuration.
     */
    const Json::Value& data() const { return m_data; }

    EntryConfig entry() const;
    TakeProfitConfig takeProfit() const;
    StopLossConfig stopLoss() const;
    BreakevenConfig breakeven() const;
    DynamicTakeProfitConfig dynamicTakeProfit() const;
    DynamicStopLossConfig dynamicStopLoss() const;

private:

    Json::Value m_data;
};

/**
 * @brief From litteral string or file JSON config file parser.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API StrategyConfig
{
public:

    StrategyConfig();
    ~StrategyConfig();

    /**
     * @brief parseDefaults Parse a char* litteral default config, from a hard coded JSON R"JSON(...)JSON" string.
     * @param litteral Char* of a correctly formatted JSON default settings.
     * @param len Len of the string or -1 to strlen.
     * @return True if successfully parsed and set into the defaultRoot.
     * @note Defaults values must be all defined.
     */
    o3d::Bool parseDefaults(const char* litteral, o3d::Int32 len=-1);

    /**
     * @brief parseStrategyOverrides Parse a JSON formatted file providing additionnals strategy settings.
     * @param basePath
     * @param filename
     * @return
     */
    o3d::Bool parseStrategyOverrides(const o3d::Dir &basePath, const o3d::String &filename);

    /**
     * @brief parseProfileOverrides Parse a JSON formatted file providing additionnals profile settings.
     * @param basePath
     * @param filename
     * @return
     */
    o3d::Bool parseProfileOverrides(const o3d::Dir &basePath, const o3d::String &filename);

    /**
     * @brief parseLearningOverrides Parse a JSON formatted file providing additionnals learning settings.
     * @param basePath
     * @param filename
     * @return
     */
    o3d::Bool parseLearningOverrides(const o3d::Dir &basePath, const o3d::String &filename);

    /**
     * @brief parseOverrides In order : strategy or profile and then learning file.
     * @param config
     * @return
     */
    o3d::Bool parseOverrides(const Config *config);

    /**
     * @brief parseMarketOverrides : Load market specific overrides.
     * @param marketId
     * @return
     */
    o3d::Bool parseMarketOverrides(const o3d::CString &marketId);

    /**
     * @brief root Strategy parameters root.
     * @return
     */
    const Json::Value& root() const { return *m_root; }

    //
    // common parameters
    //

    /**
     * @brief baseTimeframe Reference of the baseTimeframe can be double, integer or str representation.
     * Base timeframe refer to the acquired data timeframe.
     */
    o3d::Double baseTimeframe() const;

    /**
     * @brief minTradedTimeframe Reference of the minTradedTimeframe can be double, integer or str representation.
     * Base timeframe refers to the lowest traded timeframe.
     */
    o3d::Double minTradedTimeframe() const;

    /**
     * @brief maxTradedTimeframe Reference of the maxTradedTimeframe can be double, integer or str representation.
     * Base timeframe refer to the highest traded timeframe.
     */
    o3d::Double maxTradedTimeframe() const;

    /**
     * @brief tradeType Trade type to instanciate if compatible with the market capacities.
     */
    Trade::Type tradeType() const;

    //
    // helpers
    //

    o3d::Double timeframeAsDouble(Json::Value &parent, const o3d::String &key) const;
    o3d::Int32 barSizeAsInt(Json::Value &parent, const o3d::String &key) const;

    static void mergeWithDotFormat(Json::Value *root, const Json::Value &b);

private:

    Json::Value *m_root;
};

template <class T>
void configureIndicator(const AnalyserConfig &conf, const o3d::CString &name, T &indicator)
{
    Json::Value indicators = conf.indicators();

    if (indicators.isMember(name.getData())) {
        Json::Value cnf = indicators.get(name.getData(), Json::Value());
        indicator.setConf(IndicatorConfig(cnf));
    }
}

} // namespace siis

#endif // SIIS_STRATEGYCONFIG_H
