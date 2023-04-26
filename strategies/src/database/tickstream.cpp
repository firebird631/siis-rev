/**
 * @brief SiiS strategy tick stream from file.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-10
 */

#include "siis/database/tickstream.h"

#include <o3d/core/dir.h>
#include <o3d/core/file.h>
#include <o3d/core/fileinstream.h>
#include <o3d/core/filemanager.h>
#include <o3d/core/stringtokenizer.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

TickStream::TickStream(
        const o3d::String &marketPath,
        const o3d::String &brokerId,
        const o3d::String &marketId,
        const o3d::DateTime &from,
        const o3d::DateTime &to,
        o3d::Int32 bufferSize,
        Mode mode) :
    m_marketPath(marketPath),
    m_brokerId(brokerId),
    m_marketId(marketId),
    m_mode(mode),
    m_from(from),
    m_to(to),
    m_cur(from),
    m_preBuffer(bufferSize*TICK_STORED_SIZE, bufferSize*TICK_STORED_SIZE),
    m_buffer(bufferSize*TICK_MEM_SIZE, bufferSize*TICK_MEM_SIZE),
    m_bufferSize(bufferSize),
    m_finished(false),
    m_ofs(0),
    m_file(nullptr)
{
    m_fromTs = from.toDoubleTimestamp(false);
    m_toTs = to.toDoubleTimestamp(false);
}

TickStream::~TickStream()
{
    close();
}

void TickStream::open()
{
    if (m_file) {
        return;
    }

    o3d::Dir dataPath = o3d::Dir(m_marketPath);
    if (!dataPath.cd(m_brokerId)) {
        return;
    }
    if (!dataPath.cd(m_marketId)) {
        return;
    }
    if (!dataPath.cd("T")) {
        return;
    }

    // first try with binary
    if (m_mode == MODE_BINARY) {
        o3d::String filename = o3d::String("{0}{1}.dat").arg(m_cur.buildString("%Y%m")).arg(m_marketId);
        o3d::File file(dataPath.getFullPathName(), filename);
        if (file.exists()) {
            m_file = o3d::FileManager::instance()->openInStream(file.getFullFileName());
            m_curMode = MODE_BINARY;
        }
    }

    // if no binary found or ask for text try with text file
    if (!m_file) {
        // no extension
        o3d::String filename = o3d::String("{0}{1}").arg(m_cur.buildString("%Y%m")).arg(m_marketId);
        o3d::File file(dataPath.getFullPathName(), filename);
        if (file.exists()) {
            m_file = o3d::FileManager::instance()->openInStream(file.getFullFileName());
            m_curMode = MODE_TEXT;
        }
    }
}

void TickStream::close()
{
    if (m_file) {
        o3d::deletePtr(m_file);
    }
}

o3d::Int32 TickStream::fillNext(o3d::Double timestamp, DataArray &out)
{
    o3d::Int32 n = 0;

    while (!m_finished) {
        if (m_ofs >= m_buffer.getSize()) {
            // end of the buffer reached, need to parse the next bulk of data
            m_buffer.forceSize(0);
            m_ofs = 0;

            bufferize();
        }

        if (m_buffer.getSize() == 0) {
            continue;
        }

        if (m_buffer[m_ofs] < m_fromTs) {
            m_ofs += 8;  // ignore older than min timestamp (8 doubles memory size)
        } else if (m_buffer[m_ofs] > m_toTs) {
            // finished when reach max timestamp
            m_finished = true;
            close();
            break;
        } else if (m_buffer[m_ofs] <= timestamp) {
            // 8 more double for 1 tick
            out.pushArray(&m_buffer[m_ofs], 8);

            m_ofs += 8;   // one more tick of 8 doubles
            ++n;
        } else if (m_buffer[m_ofs] > timestamp) {
            break;
        }
    }

    return n;
}

o3d::Int32 TickStream::fillNext(o3d::Double timestamp, TickArray &out)
{
    o3d::Int32 n = 0;
    o3d::Int32 t = out.getSize();
    // o3d::Int32 s = 0;

    while (!m_finished) {
        if (m_ofs >= m_buffer.getSize()) {
            // end of the buffer reached, need to parse the next bulk of data
            m_buffer.forceSize(0);
            m_ofs = 0;

            bufferize();
        }

        if (m_buffer.getSize() == 0) {
            continue;
        }

        // printf("%f %f %f %f %f %f\n", m_buffer[m_ofs], m_buffer[m_ofs+1], m_buffer[m_ofs+2], m_buffer[m_ofs+3], m_buffer[m_ofs+4], m_buffer[m_ofs+5]);

        if (m_buffer[m_ofs] < m_fromTs) {
            m_ofs += 8;  // ignore older than min timestamp (8 doubles memory size)
        } else if (m_buffer[m_ofs] > m_toTs) {
            // finished when reach max timestamp
            m_finished = true;
            close();
            break;
        } else if (m_buffer[m_ofs] <= timestamp) {
            // grow output size
            if (t >= out.getMaxSize()-1) {
                out.growSize();
            }

            // 6+2 more double for 1 tick
            out.get(t)->copy(&m_buffer[m_ofs]);
            // ++s;

            m_ofs += 8;   // one more tick of 8 doubles (memory size)
            ++n;
            ++t;
        } else if (m_buffer[m_ofs] > timestamp) {
            break;
        }

//        if (s > 0) {
//            memcpy(out.getContent(out.getSize()), &m_buffer[m_ofs-s*8], s*8*sizeof(o3d::Double));
//        }
    }

    // new exact number of elements
    out.forceSize(t);
    return n;
}

//o3d::Int32 TickStream::fillNext(o3d::Double timestamp, TickArray &out)
//{
//    o3d::Int32 n = 0;

//    while (!m_finished) {
//        if (m_ofs >= m_buffer.getSize()) {
//            // end of the buffer reached, need to parse the next bulk of data
//            m_buffer.forceSize(0);
//            m_ofs = 0;

//            bufferize();
//        }

//        if (m_buffer.getSize() == 0) {
//            continue;
//        }

//        if (m_buffer[m_ofs] < m_fromTs) {
//            m_ofs += 4;  // ignore older than min timestamp
//        } else if (m_buffer[m_ofs] > m_toTs) {
//            // finished when reach max timestamp
//            m_finished = true;
//            close();
//            break;
//        } else if (m_buffer[m_ofs] <= timestamp) {
//            // grow output size
//            if (out.getSize() > 0 && n >= out.getMaxSize()-1) {
//                // save content, resize, recopy
//                // O3D_TRACE("Tick stream buffer undersized, growth x2");

//                size_t nbytes = static_cast<size_t>(n*32);
//                o3d::Double *cpy = new o3d::Double[nbytes];
//                memcpy(cpy, out.getData(), nbytes);  // its linear

//                o3d::Int32 newsize = out.getMaxSize() * 2;
//                out.resize(newsize);

//                memcpy(out.getData(), cpy, nbytes);
//                o3d::deleteArray(cpy);
//            }

//            // 4 more double for 1 tick
//            out.get(n)->copy(&m_buffer[m_ofs]);

//            m_ofs += 4;   // one more tick of 4 doubles
//            ++n;
//        } else if (m_buffer[m_ofs] > timestamp) {
//            break;
//        }
//    }

//    out.forceSize(n);
//    return n;
//}

void TickStream::bufferize()
{
    if (m_cur < m_to) {
        o3d::Bool fileEnd = false;

        if (!m_file) {
            try {
                open();
            } catch (o3d::E_InvalidResult) {
                fileEnd = true;
            } catch (o3d::E_InvalidParameter) {
                fileEnd = true;
            }
        }

        if (m_file) {
            if (m_curMode == MODE_BINARY) {
                // fill a byte pre-buffer and next convert to a buffer of doubles (per 8)
                o3d::Int32 n = m_file->read(m_preBuffer.getData(), static_cast<o3d::UInt32>(m_bufferSize*TICK_STORED_SIZE));
                m_preBuffer.forceSize(n);

                o3d::Int32 x = n / TICK_STORED_SIZE;
                o3d::Int32 ofs = 0;
                o3d::Double *d_ptr = nullptr;

                for (o3d::Int32 i = 0; i < x; ++i, ofs += TICK_STORED_SIZE) {
                    d_ptr = reinterpret_cast<o3d::Double*>(&m_preBuffer[ofs]);

                    m_buffer.push(d_ptr[0]);  // from s
                    m_buffer.push(d_ptr[1]);  // bid
                    m_buffer.push(d_ptr[2]);  // ask
                    m_buffer.push(d_ptr[3]);  // last
                    m_buffer.push(d_ptr[4]);  // volume
                    m_buffer.push(static_cast<o3d::Double>(m_preBuffer[40]));  // buy or sell

                    // align to 8 doubles
                    m_buffer.push(0.0);
                    m_buffer.push(0.0);
                }

                if (n <= 0) {
                    fileEnd = true;
                }
            } else if (m_curMode == MODE_TEXT) {
                o3d::String line;

                for (o3d::Int32 n = 0; n < m_bufferSize; ++n) {
                    if (!m_file->readLine(line)) {
                        break;
                    }

                    o3d::StringTokenizer tokenizer(line, "\t");
                    if (tokenizer.countTokens() != 6) {
                        continue;
                    }

                    if (tokenizer.hasMoreElements()) {
                        // timestamp is in millisecond, we want it in second
                        m_buffer.push(tokenizer.nextElement().toDouble() * 0.001);
                    }
                    if (tokenizer.hasMoreElements()) {
                        // bid
                        m_buffer.push(tokenizer.nextElement().toDouble());
                    }
                    if (tokenizer.hasMoreElements()) {
                        // ask
                        m_buffer.push(tokenizer.nextElement().toDouble());
                    }
                    if (tokenizer.hasMoreElements()) {
                        // last
                        m_buffer.push(tokenizer.nextElement().toDouble());
                    }
                    if (tokenizer.hasMoreElements()) {
                        // vol
                        m_buffer.push(tokenizer.nextElement().toDouble());
                    }
                    if (tokenizer.hasMoreElements()) {
                        // bos
                        m_buffer.push(static_cast<o3d::Double>(tokenizer.nextElement().toChar()));
                    }

                    // align to 8 double
                    m_buffer.push(0.0);
                    m_buffer.push(0.0);
                }
            }
        } else {
            fileEnd = true;
        }

        if (fileEnd) {
            close();

            // next month/year
            if (m_cur.month == o3d::MONTH_DECEMBER) {
                ++m_cur.year;
                m_cur.month = o3d::MONTH_JANUARY;
            } else {
                ++m_cur.month;
            }

            if (m_cur < m_to) {
                try {
                    open();
                } catch (o3d::E_InvalidResult &e) {
                    fileEnd = true;
                } catch (o3d::E_InvalidParameter &e) {
                    fileEnd = true;
                }
            }

            if (!m_file) {
                m_finished = true;
            }
        }
    } else {
        m_finished = true;
    }
}
