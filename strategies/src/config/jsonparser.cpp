/**
 * @brief SiiS strategy JSON config parser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/config/jsonparser.h"

#include <o3d/core/filemanager.h>
#include <o3d/core/file.h>
#include <o3d/core/debug.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

JsonParser::JsonParser() :
    m_root(nullptr)
{

}

JsonParser::~JsonParser()
{
    o3d::deletePtr(m_root);
}

o3d::Bool JsonParser::parse(const char *litteral, o3d::Int32 len)
{
    Json::Reader reader;
    m_root = new Json::Value();

    if (len < 0) {
        len = static_cast<o3d::Int32>(strlen(litteral));
    }

    if (!reader.parse(litteral, litteral+len, *m_root)) {
        o3d::deletePtr(m_root);
        return false;
    }

    return true;
}

o3d::Bool JsonParser::parse(const o3d::Dir &basePath, const o3d::String &filename)
{
    o3d::File file(basePath.getFullPathName(), filename);
    if (!file.exists()) {
        O3D_ERROR(o3d::E_InvalidParameter(o3d::String("Configuration file {0} not found").arg(filename)));
    }

    o3d::InStream *is = o3d::FileManager::instance()->openInStream(file.getFullFileName());
    o3d::UInt64 size = file.getFileSize();
    o3d::Char *buf = new o3d::Char[size+1];
    buf[size] = '\0';
    memset(buf, 0, size+1);

    is->reader(buf, 1, static_cast<o3d::UInt32>(size));
    o3d::deletePtr(is);

    o3d::Bool result = false;
    result = parse(buf, static_cast<o3d::Int32>(size));

    o3d::deleteArray(buf);

    return result;
}
