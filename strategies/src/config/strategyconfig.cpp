/**
 * @brief SiiS strategy config with defaults values.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/config/config.h"
#include "siis/config/strategyconfig.h"
#include "siis/utils/common.h"

#include <o3d/core/stringtokenizer.h>
#include <o3d/core/debug.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

// Recursively copy the values of b into a. Both a and b must be objects.
static void update(Json::Value& a, const Json::Value& b)
{
    if (!a.isObject() || !b.isObject()) {
        return;
    }

    for (const auto& key : b.getMemberNames()) {
        if (!a.isMember(key)) {
            continue;
        }

        if(a[key].isObject() && b[key].isObject()) {
            // common case recurse into
            update(a[key], b[key]);
        } else if(a[key].isObject() && b[key].isArray()) {
            // case of compatibility configuration for indicators
            a[key].copy(b[key]);
        } else {
            // case of leaves
            a[key].copy(b[key]);
        }
    }
}

// Find a member according to a dot formatted path and if found replace its value.
static void modifyDict(Json::Value &dict, o3d::StringTokenizer &keysOrIndexes, Json::Value newValue)
{
    Json::Value value = dict;

    while (keysOrIndexes.hasMoreTokens()) {
        o3d::String part = keysOrIndexes.nextElement();

        if (value.type() == Json::objectValue) {
            if (value.isMember(part.toAscii().getData())) {
                if (!keysOrIndexes.countTokens()) {
                    // set
                    value[part.toAscii().getData()] = newValue;
                } else {
                    value = value[part.toAscii().getData()];
                }
            } else {
                return;
            }
        } else if (value.type() == Json::arrayValue) {
            if (value.isMember(part.toAscii().getData())) {
                if (!keysOrIndexes.countTokens()) {
                    // set
                    value[part.toInt32()] = newValue;
                } else {
                    value = value[part.toInt32()];
                }
            } else {
                return;
            }
        }
    }
}

static void mergeWithDotFormat(Json::Value &root, const Json::Value &b)
{
    if (!root.isObject() || !b.isObject()) return;

    Json::Value dst = root.get("parameters", Json::Value());

    for (const auto& key : b.getMemberNames()) {
        if(b[key].type() == Json::objectValue || b[key].type() == Json::arrayValue) {
            continue;
        }

        o3d::StringTokenizer parts(o3d::String(key.c_str()), ".");
        modifyDict(dst, parts, b[key]);
    }
}

StrategyConfig::StrategyConfig() :
    m_root(nullptr)
{
    m_root = new Json::Value(Json::objectValue);
}

StrategyConfig::~StrategyConfig()
{
    o3d::deletePtr(m_root);
}

o3d::Bool StrategyConfig::parseDefaults(const char *litteral, o3d::Int32 len)
{
    JsonParser parser;

    if (parser.parse(litteral, len)) {
        *m_root = parser.root();
        return true;
    }

    return false;
}

o3d::Bool StrategyConfig::parseStrategyOverrides(const o3d::Dir &basePath, const o3d::String &filename)
{
    JsonParser parser;
    if (parser.parse(basePath, filename)) {
        update(*m_root, parser.root().get("parameters", Json::Value()));
        return true;
    }

    return false;
}

o3d::Bool StrategyConfig::parseProfileOverrides(const o3d::Dir &basePath, const o3d::String &filename)
{
    JsonParser parser;
    if (parser.parse(basePath, filename)) {
        Json::Value strategy = parser.root().get("strategy", Json::Value());
        update(*m_root, strategy.get("parameters", Json::Value()));
        return true;
    }

    return false;
}

o3d::Bool StrategyConfig::parseLearningOverrides(const o3d::Dir &basePath, const o3d::String &filename)
{
    JsonParser parser;
    if (parser.parse(basePath, filename)) {
        // watchers/connectors
        Json::Value watchers = parser.root().get("watchers", Json::Value());
        if (!watchers.empty()) {
            // compatibility format
            // @todo override symbols for each watcher
        }

        // trader/connector
        Json::Value trader = parser.root().get("trader", Json::Value());
        Json::Value markets = parser.root().get("markets", Json::Value());
        if (!trader.empty()) {
            // compatibility format
            // @todo override symbols
        } else if (!markets.empty()) {
            // @todo
            // @todo override symbols
        }

        // strategy
        Json::Value strategy = parser.root().get("strategy", Json::Value());
        Json::Value parameters = parser.root().get("parameters", Json::Value());
        if (!strategy.empty()) {
            // compatibility format
            Json::Value lparameters = strategy.get("parameters", Json::Value());
            mergeWithDotFormat(*m_root, lparameters);
        } else if (!parameters.empty()) {
            mergeWithDotFormat(*m_root, parameters);
        }

        return true;
    }

    return false;
}

o3d::Bool StrategyConfig::parseOverrides(const Config *config)
{
    if (config == nullptr) {
        return false;
    }

    // overrides
    if (config->getStrategyFilename().isValid()) {
        parseStrategyOverrides(config->getStrategiesPath(), config->getStrategyFilename());
    }
    if (config->getProfileFilename().isValid()) {
        parseProfileOverrides(config->getProfilesPath(), config->getProfileFilename());
    }
    if (config->getLearningFilename().isValid()) {
        parseLearningOverrides(config->getLearningPath(), config->getLearningFilename());
    }

    return true;
}

o3d::Double StrategyConfig::baseTimeframe() const
{
    if (m_root->get("base-timeframe", 0).isDouble() || m_root->get("base-timeframe", 0).isIntegral()) {
        return m_root->get("base-timeframe", 0).asDouble();
    } else if (m_root->get("base-timeframe", "").isString()) {
        return siis::timeframeFromStr(m_root->get("base-timeframe", "").asString().c_str());
    } else {
        return 0.0;
    }
}

o3d::Double StrategyConfig::minTradedTimeframe() const
{
    if (m_root->get("min-traded-timeframe", 0).isDouble() || m_root->get("min-traded-timeframe", 0).isIntegral()) {
        return m_root->get("min-traded-timeframe", 0).asDouble();
    } else if (m_root->get("min-traded-timeframe", "").isString()) {
        return siis::timeframeFromStr(m_root->get("min-traded-timeframe", "").asString().c_str());
    } else {
        return 0.0;
    }
}

o3d::Double StrategyConfig::maxTradedTimeframe() const
{
    if (m_root->get("max-traded-timeframe", 0).isDouble() || m_root->get("max-traded-timeframe", 0).isIntegral()) {
        return m_root->get("max-traded-timeframe", 0).asDouble();
    } else if (m_root->get("max-traded-timeframe", "").isString()) {
        return siis::timeframeFromStr(m_root->get("max-traded-timeframe", "").asString().c_str());
    } else {
        return 0.0;
    }
}

o3d::Double StrategyConfig::timeframeAsDouble(Json::Value &parent, const o3d::String &key) const
{
    if (parent.get(key.toUtf8().getData(), 0.0).isDouble() || parent.get(key.toUtf8().getData(), 0).isIntegral()) {
        return parent.get(key.toUtf8().getData(), 0.0).asDouble();
    } else if (parent.get(key.toUtf8().getData(), "").isString()) {
        return siis::timeframeFromStr(parent.get(key.toUtf8().getData(), "").asString().c_str());
    } else {
        return 0.0;
    }
}
