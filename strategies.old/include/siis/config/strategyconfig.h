/**
 * @brief SiiS strategy config with defaults values.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_STRATEGYCONFIG_H
#define SIIS_STRATEGYCONFIG_H

#include "jsonparser.h"

namespace siis {

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
     * @brief parseOverrides Parse a JSON formatted file providing additionnals settings.
     * @param basePath
     * @param filename
     * @return
     */
    o3d::Bool parseOverrides(const o3d::Dir &basePath, const o3d::String &filename);

    const Json::Value& root() const { return *m_root; }

    //
    // common parameters
    //

    /**
     * @brief minTimeframe Reference of the minTimeframe can be double, interger or str representation.
     */
    o3d::Double minTimeframe() const;

    //
    // helpers
    //

    o3d::Double timeframeAsDouble(Json::Value &parent, const o3d::String &key) const;

private:

    Json::Value *m_root;
};

template <class T>
void configureIndictor(AnalyserConfig &conf, const o3d::CString &name, T &indicator)
{
    if (conf.data().isMember(name.getData())) {
        Json::Value cnf = conf.data().get(name.getData(), Json::Value());
        indicator.setConf(IndicatorConfig(cnf));
    }
}

} // namespace siis

#endif // SIIS_STRATEGYCONFIG_H
