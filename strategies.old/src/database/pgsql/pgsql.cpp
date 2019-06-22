/**
 * @brief SiiS strategy database connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "pgsql.h"
#include <o3d/core/error.h>

#include <postgresql/libpq-fe.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "pgsqlohlcdb.h"
#include "pgsqlmarketdb.h"
#include "pgsqltradedb.h"

using namespace siis;

siis::PgSql::PgSql(
            const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &name,
            const o3d::String &user,
            const o3d::String &pwd) :
    m_conn(nullptr)
{
    o3d::String connInfo = o3d::String("hostaddr={0} port={1} dbname={2} user={3} password={4} keepalives=1")
                           .arg(host).arg(port).arg(name).arg(user).arg(pwd);
    // PGresult *res;
    m_conn = PQconnectdb(connInfo.toUtf8().getData());

    if (PQstatus(m_conn) != CONNECTION_OK) {
        const char* err = PQerrorMessage(m_conn);
        o3d::String msg;
        msg.fromUtf8(err);

        throw o3d::E_InvalidResult(msg);
    }
}

siis::PgSql::~PgSql()
{
    if (m_conn) {
        PQfinish(m_conn);
        m_conn = nullptr;
    }
}

OhlcDb *siis::PgSql::buildOhlcDb()
{
    return new PgSqlOhlcDb(this);
}

MarketDb *siis::PgSql::buildMarketDb()
{
    return new PgSqlMarketDb(this);
}

TradeDb *siis::PgSql::buildTradeDb()
{
    return new PgSqlTradeDb(this);
}

//show_binary_results(PGresult *res)
//{
//    int         i,
//                j;
//    int         i_fnum,
//                t_fnum,
//                b_fnum;

//    /* Use PQfnumber to avoid assumptions about field order in result */
//    i_fnum = PQfnumber(res, "i");
//    t_fnum = PQfnumber(res, "t");
//    b_fnum = PQfnumber(res, "b");

//    for (i = 0; i < PQntuples(res); i++)
//    {
//        char       *iptr;
//        char       *tptr;
//        char       *bptr;
//        int         blen;
//        int         ival;

//        /* Get the field values (we ignore possibility they are null!) */
//        iptr = PQgetvalue(res, i, i_fnum);
//        tptr = PQgetvalue(res, i, t_fnum);
//        bptr = PQgetvalue(res, i, b_fnum);

//        /*
//         * The binary representation of INT4 is in network byte order, which
//         * we'd better coerce to the local byte order.
//         */
//        ival = ntohl(*((uint32_t *) iptr));

//        /*
//         * The binary representation of TEXT is, well, text, and since libpq
//         * was nice enough to append a zero byte to it, it'll work just fine
//         * as a C string.
//         *
//         * The binary representation of BYTEA is a bunch of bytes, which could
//         * include embedded nulls so we have to pay attention to field length.
//         */
//        blen = PQgetlength(res, i, b_fnum);

//        printf("tuple %d: got\n", i);
//        printf(" i = (%d bytes) %d\n",
//               PQgetlength(res, i, i_fnum), ival);
//        printf(" t = (%d bytes) '%s'\n",
//               PQgetlength(res, i, t_fnum), tptr);
//        printf(" b = (%d bytes) ", blen);
//        for (j = 0; j < blen; j++)
//            printf("\\%03o", bptr[j]);
//        printf("\n\n");
//    }
//}

//    /* Here is our out-of-line parameter value */
//    paramValues[0] = "joe's place";

//    res = PQexecParams(conn,
//                       "SELECT * FROM test1 WHERE t = $1",
//                       1,       /* one param */
//                       NULL,    /* let the backend deduce param type */
//                       paramValues,
//                       NULL,    /* don't need param lengths since text */
//                       NULL,    /* default to all text params */
//                       1);      /* ask for binary results */

//    if (PQresultStatus(res) != PGRES_TUPLES_OK)
//    {
//        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
//        PQclear(res);
//        exit_nicely(conn);
//    }

//    show_binary_results(res);

//    PQclear(res);

// https://www.postgresql.org/docs/9.4/libpq-connect.html
// https://docs.postgresql.fr/8.1/libpq-example.html

void PgSql::terminate()
{
    // @todo
}
