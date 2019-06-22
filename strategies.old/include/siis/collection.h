/**
 * @brief SiiS strategies and training modules collection.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_COLLECTION_H
#define SIIS_COLLECTION_H

#include <o3d/core/dir.h>
#include <o3d/core/dynamiclibrary.h>
#include <o3d/core/stringmap.h>

#include "strategy.h"
#include "learning/supervisor.h"

namespace siis {

typedef Strategy* (*StrategyPluginFunction)(Handler *handler, const o3d::String &identifier);

/**
 * @brief List detected strategies and training modules and builder.
 * @author Frederic Scherma
 * @date 2019-03-05
 */
class StrategyCollection
{
public:

    StrategyCollection(const o3d::String &basePath);
    ~StrategyCollection();

    void init();
    void terminate();

    bool has(const o3d::String &name) const;

    /**
     * @brief builder Create a new strategy instance with a unique identifier
     * @param handler Valid strategy handler.
     * @param strategy Valid strategy type name.
     * @param identifier Unique strategy identifer (ie: "<broker-id>.<market-id>").
     * @return A new strategy instance or null
     * @throw E_InvalidParameter
     */
    Strategy* build(Handler *handler, const o3d::String &name, const o3d::String &identifier);

    o3d::DynamicLibrary library() { return *m_library; }
    const o3d::DynamicLibrary library() const { return *m_library; }

private:

    o3d::String m_basePath;
    o3d::StringMap<o3d::String> m_collection;  //! Name as key, .so fullpath as value
    o3d::DynamicLibrary *m_library;
};

typedef Supervisor* (*SupervisorPluginFunction)(Handler *handler, const o3d::String &identifier);

/**
 * @brief List detected supervisors training modules and builder.
 * @author Frederic Scherma
 * @date 2019-03-05
 */
class SupervisorCollection
{
public:

    SupervisorCollection(const o3d::String &basePath);
    ~SupervisorCollection();

    void init();
    void terminate();

    bool has(const o3d::String &name) const;

    /**
     * @brief builder Create a new supervisor instance with a unique identifier
     * @param handler Valid supervisor handler.
     * @param strategy Valid supervisor type name.
     * @param identifier Unique supervisor identifer.
     * @return A new supervisor instance or null
     * @throw E_InvalidParameter
     */
    Supervisor* build(Handler *handler, const o3d::String &name, const o3d::String &identifier);

    o3d::DynamicLibrary library() { return *m_library; }
    const o3d::DynamicLibrary library() const { return *m_library; }

private:

    o3d::String m_basePath;
    o3d::StringMap<o3d::String> m_collection;  //! Name as key, .so fullpath as value
    o3d::DynamicLibrary *m_library;
};

} // namespace siis

#endif // SIIS_COLLECTION_H
