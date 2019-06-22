/**
 * @brief SiiS strategy database connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/database/database.h"
#include <o3d/core/error.h>

#include "mysql/mysql.h"
#include "pgsql/pgsql.h"

using namespace siis;

Database *Database::builder(
        const o3d::String &db,
        const o3d::String &host,
        o3d::UInt32 port,
        const o3d::String &name,
        const o3d::String &user,
        const o3d::String &pwd)
{
    if (db == "mysql") {
        return new siis::MySql(host, port, name, user, pwd);
    } else if (db == "postgresql") {
        return new siis::PgSql(host, port, name, user, pwd);
    } else {
        throw o3d::E_InvalidParameter("Unsupported DB type");
    }
}

Database::~Database()
{

}
