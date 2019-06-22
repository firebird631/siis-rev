/**
 * @brief SiiS strategy ohlc stream from database.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "siis/database/ohlcstream.h"

#include <o3d/core/dir.h>
#include <o3d/core/file.h>
#include <o3d/core/fileinstream.h>
#include <o3d/core/filemanager.h>
#include <o3d/core/stringtokenizer.h>

#include "siis/database/database.h"
#include "siis/database/ohlcdb.h"

using namespace siis;
using o3d::Debug;
using o3d::Logger;

OhlcStream::OhlcStream(
        Database *db,
        const o3d::String &marketPath,
        const o3d::String &brokerId,
        const o3d::String &marketId,
        const o3d::DateTime &from,
        const o3d::DateTime &to,
        o3d::Int32 bufferSize) :
    m_marketPath(marketPath),
    m_brokerId(brokerId),
    m_marketId(marketId),
    m_from(from),
    m_to(to),
    m_cur(from),
    m_buffer(bufferSize*8, bufferSize*8),
    m_bufferSize(bufferSize),
    m_finished(false),
    m_ofs(0),
    m_ohlcDb(nullptr)
{
    O3D_ASSERT(db != nullptr);

    m_fromTs = from.toDoubleTimestamp();
    m_toTs = to.toDoubleTimestamp();

    // @todo build a new ohlcDb from db
}

OhlcStream::~OhlcStream()
{
    close();
}

void OhlcStream::open()
{
    if (m_ohlcDb) {
        return;
    }
}

void OhlcStream::close()
{
    if (m_ohlcDb) {
        o3d::deletePtr(m_ohlcDb);
    }
}

o3d::Int32 OhlcStream::fillNext(o3d::Double timestamp, DataArray &out)
{
    o3d::Int32 n = 0;

    // @todo

    return n;
}

o3d::Int32 OhlcStream::fillNext(o3d::Double timestamp, OhlcArray &out)
{
    o3d::Int32 n = 0;
    o3d::Int32 t = out.getSize();

    // @todo

    // new exact number of elements
    out.forceSize(t);
    return n;
}

void OhlcStream::bufferize()
{
    if (m_cur < m_to) {
        if (!m_ohlcDb) {
            open();
        }

        o3d::Bool rowsEnd = false;

        if (m_ohlcDb) {
            // @todo results from row to buffer
            rowsEnd = true;
        }

        if (rowsEnd) {
            // next month/year
            if (m_cur.month == o3d::MONTH_DECEMBER) {
                ++m_cur.year;
                m_cur.month = o3d::MONTH_JANUARY;
            } else {
                ++m_cur.month;
            }

            if (m_cur < m_to) {
                // @todo next query
            }

            if (!m_ohlcDb) {
                m_finished = true;
            }
        }
    } else {
        m_finished = true;
    }
}
