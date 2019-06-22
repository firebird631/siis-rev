/**
 * @brief SiiS strategy collection.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/collection.h"

#include <o3d/core/filemanager.h>
#include <o3d/core/file.h>
#include <o3d/core/localdir.h>
#include <o3d/core/error.h>
#include <o3d/core/stringlist.h>

using namespace siis;

StrategyCollection::StrategyCollection(const o3d::String &basePath) :
    m_basePath(basePath),
    m_library(nullptr)
{

}

StrategyCollection::~StrategyCollection()
{

}

void StrategyCollection::init()
{
    o3d::LocalDir dir(m_basePath);
    if (!dir.isAbsolute()) {
        dir.makeAbsolute();
    }

    o3d::Int32 pos = 0;
    o3d::T_StringList plugins = dir.findFiles("*.so", o3d::FILE_FILE);
    for (o3d::String plugin : plugins) {
        o3d::String name(plugin);

        if (name.startsWith("lib")) {
            name.remove(0, 3);
        }

        pos = name.reverseFind('.');
        if (pos > 0) {
            name.truncate(pos);
            m_collection[name] = dir.getFullPathName() + '/' + plugin;
        }
    }
}

bool StrategyCollection::has(const o3d::String &name) const
{
    return m_collection.find(name) != m_collection.cend();
}

Strategy* StrategyCollection::build(Handler *handler, const o3d::String &name, const o3d::String &identifier)
{
    Strategy *result = nullptr;

    auto it = m_collection.find(name);
    if (it != m_collection.cend()) {
        o3d::DynamicLibrary *lib = o3d::DynamicLibrary::load(it->second);

        if (!lib) {
            throw o3d::E_InvalidParameter(o3d::String("Strategy {0} cannot be loaded").arg(name));
        }

        StrategyPluginFunction *fptr = static_cast<StrategyPluginFunction*>(lib->getFunctionPtr("siisStrategy"));
        if (!fptr) {
            o3d::DynamicLibrary::unload(lib);
            throw o3d::E_InvalidParameter(o3d::String("Strategy {0} library has no siisStrategy function").arg(name));
        }

        result = reinterpret_cast<StrategyPluginFunction>(fptr)(handler, identifier);
        if (!result) {
            o3d::DynamicLibrary::unload(lib);
            throw o3d::E_InvalidParameter(o3d::String("Strategy {0} library cannot be instantiate").arg(name));
        }
    } else {
        throw o3d::E_InvalidParameter(o3d::String("Strategy {0} not found").arg(name));
    }

    return result;
}

void StrategyCollection::terminate()
{
    if (m_library) {
        o3d::DynamicLibrary::unload(m_library);
        m_library = nullptr;
    }
}

//
// supervisor
//

SupervisorCollection::SupervisorCollection(const o3d::String &basePath) :
    m_basePath(basePath),
    m_library(nullptr)
{

}

SupervisorCollection::~SupervisorCollection()
{

}

void SupervisorCollection::init()
{
    o3d::LocalDir dir(m_basePath);
    if (!dir.isAbsolute()) {
        dir.makeAbsolute();
    }

    o3d::Int32 pos = 0;
    o3d::T_StringList plugins = dir.findFiles("*.so", o3d::FILE_FILE);
    for (o3d::String plugin : plugins) {
        o3d::String name(plugin);

        if (name.startsWith("lib")) {
            name.remove(0, 3);
        }

        pos = name.reverseFind('.');
        if (pos > 0) {
            name.truncate(pos);
            m_collection[name] = dir.getFullPathName() + '/' + plugin;
        }
    }
}

bool SupervisorCollection::has(const o3d::String &name) const
{
    return m_collection.find(name) != m_collection.cend();
}

Supervisor* SupervisorCollection::build(Handler *handler, const o3d::String &name, const o3d::String &identifier)
{
    Supervisor *result = nullptr;

    auto it = m_collection.find(name);
    if (it != m_collection.cend()) {
        o3d::DynamicLibrary *lib = o3d::DynamicLibrary::load(it->second);

        if (!lib) {
            throw o3d::E_InvalidParameter(o3d::String("Supervisor {0} cannot be loaded").arg(name));
        }

        StrategyPluginFunction *fptr = static_cast<StrategyPluginFunction*>(lib->getFunctionPtr("siisStrategy"));
        if (!fptr) {
            o3d::DynamicLibrary::unload(lib);
            throw o3d::E_InvalidParameter(o3d::String("Supervisor {0} library has no siisStrategy function").arg(name));
        }

        result = reinterpret_cast<SupervisorPluginFunction>(fptr)(handler, identifier);
        if (!result) {
            o3d::DynamicLibrary::unload(lib);
            throw o3d::E_InvalidParameter(o3d::String("Supervisor {0} library cannot be instantiate").arg(name));
        }
    } else {
        throw o3d::E_InvalidParameter(o3d::String("Supervisor {0} not found").arg(name));
    }

    return result;
}

void SupervisorCollection::terminate()
{
    if (m_library) {
        o3d::DynamicLibrary::unload(m_library);
        m_library = nullptr;
    }
}
