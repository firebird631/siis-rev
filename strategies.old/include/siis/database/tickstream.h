/**
 * @brief SiiS strategy tick stream from file.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-10
 */

#ifndef SIIS_TICKSTREAM_H
#define SIIS_TICKSTREAM_H

#include "../base.h"
#include "../constants.h"
#include "../tick.h"
#include "../dataarray.h"

#include <o3d/core/templatearray.h>
#include <o3d/core/instream.h>
#include <o3d/core/datetime.h>

namespace siis {

/**
 * @brief SiiS strategy tick stream from file.
 * @author Frederic Scherma
 * @date 2019-03-10
 * @todo stream using a list of multiples markets in case of futurs contracts
 */
class SIIS_API TickStream
{
public:

    enum Mode
    {
        MODE_BINARY = 0,
        MODE_TEXT = 1
    };

    TickStream(
            const o3d::String &marketPath,
            const o3d::String &brokerId,
            const o3d::String &marketId,
            const o3d::DateTime &from,
            const o3d::DateTime &to,
            o3d::Int32 bufferSize = 8192,  //!< multiplied by 4 float per tick
            Mode mode = MODE_BINARY);

    ~TickStream();

    const o3d::String& brokerId() const { return m_brokerId; }
    const o3d::String& marketId() const { return m_marketId; }

    /**
     * @brief mode Prefered mode Binary/Text (fallback to the other if not available).
     */
    Mode mode() const { return m_mode; }

    /**
     * @brief fillNext Fill block of 4 double until timestamp is reached.
     * @param timestamp Limit timestamp to reach (inclusive).
     * @param out Array where to append the new values.
     * @return Number of filled ticks.
     */
    o3d::Int32 fillNext(o3d::Double timestamp, DataArray &out);

    /**
     * @brief fillNext Fill block of 4 double until timestamp is reached in an optimized tick atomic array.
     * @param timestamp Limit timestamp to reach (inclusive).
     * @param out Array where to append the new values.
     * @return Number of filled ticks.
     */
    o3d::Int32 fillNext(o3d::Double timestamp, TickArray &out);


    o3d::Bool finished() const { return (m_cur >= m_to) && m_finished; }

private:

    void open();
    void close();

    o3d::String m_marketPath;
    o3d::String m_brokerId;
    o3d::String m_marketId;

    Mode m_mode;
    Mode m_curMode;

    o3d::DateTime m_from;
    o3d::DateTime m_to;
    o3d::DateTime m_cur;

    o3d::Double m_fromTs;
    o3d::Double m_toTs;

    DataArray m_buffer;
    o3d::Int32 m_bufferSize;

    o3d::Bool m_finished;
    o3d::Int32 m_ofs;

    o3d::InStream *m_file;

    void bufferize();
};

} // namespace siis

#endif // SIIS_TICKSTREAM_H
