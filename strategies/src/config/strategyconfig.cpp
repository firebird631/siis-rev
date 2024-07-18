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

static void update(Json::Value& a, const Json::Value& b);

static void joinRightToLeft(Json::Value& a, const Json::Value& b)
{
    for (const auto& key : b.getMemberNames()) {
        if (!a.isMember(key)) {
            // original from user
            a[key].copy(b[key]);
        } else {
            // update from default
            // update(a[key], b[key]);
            // take user version
            a[key].copy(b[key]);
        }
    }

    for (const auto& key : a.getMemberNames()) {
        if (!b.isMember(key)) {
            // remove from dst
            a.removeMember(key);
        }
    }
}

static void mergeRightToLeft(Json::Value& a, const Json::Value& b)
{
    for (const auto& key : b.getMemberNames()) {
        if (!a.isMember(key)) {
            // original from user
            a[key].copy(b[key]);
        } else {
            // update from default
             update(a[key], b[key]);
        }
    }

    for (const auto& key : a.getMemberNames()) {
        if (!b.isMember(key)) {
            // remove from dst
            a.removeMember(key);
        }
    }
}

// Recursively copy the values of b into a. Both a and b must be objects.
static void update(Json::Value& a, const Json::Value& b)
{
    if (!a.isObject() || !b.isObject()) {
        return;
    }

    for (const auto& key : b.getMemberNames()) {
        if (key == "timeframes") {
            // special case
            joinRightToLeft(a[key], b[key]);
            continue;
        } else if (key == "contexts") {
            // special case
            joinRightToLeft(a[key], b[key]);
            continue;
        }

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
static void modifyDict(Json::Value *root, o3d::StringTokenizer &keysOrIndexes, Json::Value newValue)
{
    Json::Value *value = root;

    while (keysOrIndexes.hasMoreTokens()) {
        o3d::String part = keysOrIndexes.nextElement();
        if (value->isArray()) {
            if (!keysOrIndexes.countTokens()) {
                // check
                o3d::Int32 idx = part.toInt32();
                if (idx >= 0 && idx < value->size()) {
                    // set
                    (*value)[idx].copy(newValue);
                } else {
                    return;
                }
            } else {
                // check
                o3d::Int32 idx = part.toInt32();
                if (idx >= 0 && idx < value->size()) {
                    value = &value[part.toInt32()];
                } else {
                    return;
                }
            }
        } else if (value->isObject()) {
            if (value->isMember(part.toAscii().getData())) {
                if (!keysOrIndexes.countTokens()) {
                    // set
                    (*value)[part.toAscii().getData()].copy(newValue);
                } else {
                    value = &(*value)[part.toAscii().getData()];
                }
            } else {
                return;
            }
        }
    }
}

static void mergeWithDotFormat(Json::Value *root, const Json::Value &b)
{
    if (!root->isObject() || !b.isObject()) return;

    for (const auto& key : b.getMemberNames()) {
        if(b[key].type() == Json::objectValue || b[key].type() == Json::arrayValue) {
            continue;
        }

        o3d::StringTokenizer parts(o3d::String(key.c_str()), ".");
        modifyDict(root, parts, b[key]);
    }
}

o3d::Int32 AnalyserConfig::getAnalyserInt(const o3d::CString &analyserName,
                                          const o3d::CString &valueName,
                                          o3d::Int32 defValue) const
{
    if (indicators().isMember(analyserName.getData())) {
        Json::Value cnf = indicators().get(analyserName.getData(), Json::Value());
        if (cnf.isMember(valueName.getData())) {
            return cnf.get(valueName.getData(), defValue).asInt();
        }
    }

    return defValue;
}

o3d::Double AnalyserConfig::getAnalyserDouble(const o3d::CString &analyserName,
                                              const o3d::CString &valueName,
                                              o3d::Double defValue) const
{
    if (indicators().isMember(analyserName.getData())) {
        Json::Value cnf = indicators().get(analyserName.getData(), Json::Value());
        if (cnf.isMember(valueName.getData())) {
            return cnf.get(valueName.getData(), defValue).asDouble();
        }
    }

    return defValue;
}

Trade::Type tradeTypeFromStr(const o3d::String &type)
{
    if (type == "spot" || type == "asset" || type == "buysell") {
        return Trade::TYPE_BUY_SELL;
    } else if (type == "margin") {
        return Trade::TYPE_MARGIN;
    } else if (type == "ind-margin") {
        return Trade::TYPE_IND_MARGIN;
    } else if (type == "position") {
        return Trade::TYPE_POSITION;
    } else {
        O3D_ERROR(o3d::E_InvalidParameter(o3d::String("{0} is not a valid trade-type").arg(type)));
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
            mergeWithDotFormat(m_root, lparameters);
        } else if (!parameters.empty()) {
            mergeWithDotFormat(m_root, parameters);
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

Trade::Type StrategyConfig::tradeType() const
{
    if (m_root->isMember("trade-type")) {
        o3d::String tradeType = m_root->get("trade-type", "asset").asString().c_str();
        return tradeTypeFromStr(tradeType);
    }

    return Trade::TYPE_ASSET;
}

o3d::Double StrategyConfig::timeframeAsDouble(Json::Value &parent, const o3d::String &key) const
{
    if (parent.get(key.toUtf8().getData(), 0.0).isDouble() || parent.get(key.toUtf8().getData(), 0).isIntegral()) {
        return parent.get(key.toUtf8().getData(), 0.0).asDouble();
    } else if (parent.get(key.toUtf8().getData(), "").isString()) {
        return siis::timeframeFromStr(parent.get(key.toUtf8().getData(), "0").asString().c_str());
    } else {
        return 0.0;
    }
}

o3d::Int32 StrategyConfig::barSizeAsInt(Json::Value &parent, const o3d::String &key) const
{
    if (parent.get(key.toUtf8().getData(), 0.0).isDouble() || parent.get(key.toUtf8().getData(), 0).isIntegral()) {
        return static_cast<o3d::Int32>(parent.get(key.toUtf8().getData(), 0).asInt64());
    } else if (parent.get(key.toUtf8().getData(), "").isString()) {
        return o3d::String(parent.get(key.toUtf8().getData(), "0").asString().c_str()).toInt32();
    } else {
        return 0;
    }
}

EntryConfig ContextConfig::entry() const
{
    if (data().isMember("entry")) {
        Json::Value d = data().get("entry", Json::Value());
        return EntryConfig(d);
    }

    return EntryConfig(Json::Value());
}

TakeProfitConfig ContextConfig::takeProfit() const
{
    if (data().isMember("take-profit")) {
        Json::Value d = data().get("take-profit", Json::Value());
        return TakeProfitConfig(d);
    }

    return TakeProfitConfig(Json::Value());
}

StopLossConfig ContextConfig::stopLoss() const
{
    if (data().isMember("stop-loss")) {
        Json::Value d = data().get("stop-loss", Json::Value());
        return StopLossConfig(d);
    }

    return StopLossConfig(Json::Value());
}

BreakevenConfig ContextConfig::breakeven() const
{
    if (data().isMember("breakeven")) {
        Json::Value d = data().get("breakeven", Json::Value());
        return BreakevenConfig(d);
    }

    return BreakevenConfig(Json::Value());
}

DynamicTakeProfitConfig ContextConfig::dynamicTakeProfit() const
{
    if (data().isMember("dynamic-take-profit")) {
        Json::Value d = data().get("dynamic-take-profit", Json::Value());
        return DynamicTakeProfitConfig(d);
    }

    return DynamicTakeProfitConfig(Json::Value());
}

DynamicStopLossConfig ContextConfig::dynamicStopLoss() const
{
    if (data().isMember("dynamic-stop-loss")) {
        Json::Value d = data().get("dynamic-stop-loss", Json::Value());
        return DynamicStopLossConfig(d);
    }

    return DynamicStopLossConfig(Json::Value());
}
