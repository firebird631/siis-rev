/**
 * @brief SiiS strategy database connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_PGSQL_H
#define SIIS_PGSQL_H

#include "siis/database/database.h"
#include <o3d/core/database.h>

extern "C"
{
typedef struct pg_conn PGconn;
}

namespace siis {

/**
 * @brief Strategy computer.
 * @author Frederic Scherma
 * @date 2019-03-07
 */
class SIIS_API PgSql : public siis::Database
{
public:

    PgSql(const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &name,
            const o3d::String &user,
            const o3d::String &pwd);

    virtual ~PgSql();

    virtual void init();
    virtual void terminate();

    virtual o3d::Int32 run(void *);

private:

    o3d::Database *m_db;
    PGconn *m_conn;
};

} // namespace siis

#endif // SIIS_PGSQL_H
