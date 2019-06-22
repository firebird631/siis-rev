/**
 * @brief SiiS strategy config with defaults values.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/config/strategyconfig.h"
#include "siis/utils/common.h"

#include <o3d/core/debug.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

// Recursively copy the values of b into a. Both a and b must be objects.
static void update(Json::Value& a, const Json::Value& b) {
    if (!a.isObject() || !b.isObject()) return;

    for (const auto& key : b.getMemberNames()) {
        //if (a[key].isObject()) {
        if(a[key].type() == Json::objectValue && b[key].type() == Json::objectValue) {
            update(a[key], b[key]);
        } else {
            a[key] = b[key];
        }
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

o3d::Bool StrategyConfig::parseOverrides(const o3d::Dir &basePath, const o3d::String &filename)
{
    JsonParser parser;
    if (parser.parse(basePath, filename)) {
        update(*m_root, parser.root().get("parameters", Json::Value()));
        return true;
    }

    return false;
}

o3d::Double StrategyConfig::minTimeframe() const
{
    if (m_root->get("minTimeframe", 0).isDouble() || m_root->get("minTimeframe", 0).isIntegral()) {
        return m_root->get("minTimeframe", 0).asDouble();
    } else if (m_root->get("minTimeframe", "").isString()) {
        return siis::timeframeFromStr(m_root->get("minTimeframe", "").asString().c_str());
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
