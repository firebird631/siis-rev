/**
 * @brief SiiS strategy database connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_MYSQL_H
#define SIIS_MYSQL_H

#include "siis/database/database.h"
#include <o3d/core/database.h>

namespace siis {

/**
 * @brief Strategy computer.
 * @author Frederic Scherma
 * @date 2019-03-07
 */
class SIIS_API MySql : public siis::Database
{
public:

    MySql(const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &name,
            const o3d::String &user,
            const o3d::String &pwd);

    virtual ~MySql();

    virtual void init();
    virtual void terminate();

    virtual o3d::Int32 run(void *);

private:

    o3d::Database *m_db;
};

} // namespace siis

#endif // SIIS_MYSQL_H
