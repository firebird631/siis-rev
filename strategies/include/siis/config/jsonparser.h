/**
 * @brief SiiS strategy JSON config parser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_JSONPARSER_H
#define SIIS_JSONPARSER_H

#include "../base.h"

#include <o3d/core/dir.h>
#include <o3d/core/stringmap.h>
#include <o3d/core/configfile.h>
#include <o3d/core/stringlist.h>

#include <json/reader.h>
#include <json/value.h>

namespace siis {

/**
 * @brief From litteral string or file JSON config file parser.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API JsonParser
{
public:

    JsonParser();
    ~JsonParser();

    o3d::Bool parse(const char* litteral, o3d::Int32 len=-1);
    o3d::Bool parse(const o3d::Dir &basePath, const o3d::String &filename);

    o3d::Bool isValid() const { return m_root != nullptr; }
    const Json::Value& root() const { return *m_root; }

private:

    Json::Value *m_root;
};

} // namespace siis

#endif // SIIS_JSONPARSER_H
