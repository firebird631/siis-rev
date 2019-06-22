/**
 * @brief SiiS strategy bid or mid or ofr OHLC model and array.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_OHLC_H
#define SIIS_OHLC_H

#include "base.h"

#include <o3d/core/atomicarray.h>
#include <o3d/core/base.h>
#include <o3d/core/memorydbg.h>

using o3d::MemoryManager;

namespace siis {

/**
 * @brief SiiS strategy bid or mid or ofr OHLC model and array.
 * @author Frederic Scherma
 * @date 2019-03-07
 * It contain 8 double values arranged : timestamp, timeframe, open, high, low, close, volume, ended.
 * Data are 16 bytes memory aligned.
 */
class SIIS_API Ohlc
{
public:

    enum Type {
        TYPE_MID = 0,
        TYPE_BID = 1,
        TYPE_OFR = 2,
        MAX_TYPE = TYPE_OFR
    };

    static const o3d::Int32 NUM_TYPE = MAX_TYPE+1;

    //! Atomic array initialization and set to identity
    inline void _initAtomicObject(void *ptr)
    {
        D = reinterpret_cast<o3d::Double*>(ptr);
        zero();
    }

    inline void zero()
    {
        memset(D, 0, 8*sizeof(o3d::Double));
    }

    Ohlc()
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(64));
        zero();
    }

    Ohlc(o3d::Double ts, o3d::Double tf,
         o3d::Double o, o3d::Double h, o3d::Double l, o3d::Double c,
         o3d::Double v, o3d::Double ended=1.0)
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(64));
        D[0] = ts;
        D[1] = tf;
        D[2] = o;
        D[3] = h;
        D[4] = l;
        D[5] = c;
        D[6] = v;
        D[7] = ended;
    }

    Ohlc(o3d::Double *d)
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(64));
        memcpy(D, d, 8*sizeof(o3d::Double));
    }

    Ohlc(o3d::Double tf, o3d::Double *d, o3d::Double ended=1.0)
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(64));
        D[0] = d[0];
        D[1] = tf;
        D[2] = d[1];
        D[3] = d[2];
        D[4] = d[3];
        D[5] = d[4];
        D[6] = d[5];
        D[7] = ended;
    }

    Ohlc(const Ohlc &d)
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(64));
        memcpy(D, d.D, 8*sizeof(o3d::Double));
    }

	inline void copy(const o3d::Double *d)
	{
		memcpy(D, d, 8 * sizeof(o3d::Double));
	}

    ~Ohlc() { O3D_FAST_FREE(D, 64); }

    inline o3d::Double timestamp() const { return D[0]; }
    inline o3d::Double ts() const { return D[0]; }

    inline o3d::Double timeframe() const { return D[1]; }
    inline o3d::Double tf() const { return D[1]; }

    inline o3d::Double open() const { return D[2]; }
    inline o3d::Double o() const { return D[2]; }

    inline o3d::Double high() const { return D[3]; }
    inline o3d::Double h() const { return D[3]; }

    inline o3d::Double low() const { return D[4]; }
    inline o3d::Double l() const { return D[4]; }

    inline o3d::Double close() const { return D[5]; }
    inline o3d::Double c() const { return D[5]; }

    inline o3d::Double volume() const { return D[6]; }
    inline o3d::Double vol() const { return D[6]; }
    inline o3d::Double v() const { return D[6]; }

    inline o3d::Bool consolidated() const { return D[7] != 0.0; }
    inline o3d::Bool ended() const { return D[7] != 0.0; }
    inline o3d::Bool e() const { return D[7] != 0.0; }

    inline o3d::Double height() const { return D[3] - D[4]; }

    inline void setTimestamp(o3d::Double ts) { D[0] = ts; }
    inline void setTimeframe(o3d::Double tf) { D[1] = tf; }

    inline void setOhlc(o3d::Double o) { D[2] = D[3] = D[4] = D[5] = o; }

    inline void setOhlc(o3d::Double o, o3d::Double h, o3d::Double l, o3d::Double c)
    {
        D[2] = o; D[3] = h; D[4] = l; D[5] = c;
    }

    inline void setO(o3d::Double o) { D[2] = o; }
    inline void setH(o3d::Double h) { D[3] = h; }
    inline void setL(o3d::Double l) { D[4] = l; }
    inline void setC(o3d::Double c) { D[5] = c; }

    inline void setVolume(o3d::Double v) { D[6] = v; }

    /**
     * @brief setConsolidated Set to consolidate/close/terminate the status of this ohlc.
     */
    inline void setConsolidated() { D[7] = 1.0; }

    inline o3d::String toString() const
    {
        return o3d::String("ts={0}, tf={1}, ohlc={2}/{3}/{4}/{5}, vol={6}, ended={7}").arg(D[0]).arg(D[1]).arg(D[2]).
                arg(D[3]).arg(D[4]).arg(D[5]).arg(D[6]).arg(D[7]);
    }

private:

    o3d::Double *D;
};

/**
 * @brief OhlcArray Ohlc 16 bytes memory aligned array for data, with object management and a growth capacity.
 */
class SIIS_API OhlcArray : public o3d::AtomicArray<Ohlc, o3d::Double, 6>
{
public:

    //! default constructor (no elements)
    OhlcArray() : AtomicArray() {}

    /**
     * @brief OhlcArray Initialization constructor
     * @param NbElt Number of element to allocate.
     */
    OhlcArray(o3d::Int32 nbElt) : AtomicArray(nbElt)
    {
        forceSize(0);  // initially 0 element are used
    }

    /**
     * @brief growSize Double the current size with conservation of the current values at the same positions.
     */
    void growSize()
    {
        if (getMaxSize() == 0) {
            // empty then alloc a minimal of 100 elements
            resize(100);
            forceSize(0);
        } else {
            // save content, resize, recopy
            size_t nbytes = static_cast<size_t>(getSize()*64);
            o3d::Double *cpy = new o3d::Double[static_cast<size_t>(getSize()*8)];
            memcpy(cpy, getData(), nbytes);  // its linear

            o3d::Int32 prevSize = getSize();
            o3d::Int32 newSize = getMaxSize() * 2;
            resize(newSize);

            forceSize(prevSize);

            memcpy(getData(), cpy, nbytes);
            o3d::deleteArray(cpy);
        }
    }

    /**
     * @brief clear Set to 0 element but keep the array allocation.
     */
    void clear() { forceSize(0); }

    /**
     * @brief push Push back a tick, growth of the array size if necessary.
     */
    inline void push(const Ohlc &ohlc)
    {
        o3d::Int32 t = getSize();

        // grow output size
        if (getSize() > 0 && t >= getMaxSize()-1) {
            growSize();
        }

        *get(t) = ohlc;
    }
};

/**
 * @brief OhlcCircular Circular array of ohlc 16 bytes memory aligned array for data, with object management.
 */
class SIIS_API OhlcCircular : public o3d::AtomicArray<Ohlc, o3d::Double, 6>
{
public:

    struct Cit
    {
        Cit(const OhlcCircular *b, const Ohlc *p) : b(b), p(p) {}

        o3d::Bool operator!= (const Cit &_which) const
        {
            return  p != _which.p;
        }

        Cit& operator++ ()
        {
            ++p;

            if (p == b->m_last) {
                p = nullptr;
            }

            if (p == b->m_end) {
                p = b->get(0);

                if (p == b->m_last) {
                    p = nullptr;
                }
            }

            return *this;
        }

        const Ohlc* operator* () const
        {
            return p;
        }

        const OhlcCircular *b;
        const Ohlc *p;
    };

    /**
     * @brief OhlcCircular Initialization constructor
     * @param size Number of element to allocate.
     */
    OhlcCircular(o3d::Int32 size) :
        AtomicArray(size),
        m_first(get(0)),
        m_last(get(0)),
        m_end(get(0)+size),
        m_size(0)
    {
        O3D_ASSERT(size > 1);  // at least 2 elements
    }

    /**
     * @brief clear Set to 0 element but keep the array allocation.
     */
    inline void clear()
    {
        m_first = m_last = get(0);
        m_size = 0;
    }

    /**
     * @brief writeElt Return the current writable ohlc and move to the next position.
     * @return A valid pointer on the last ohlc for write.
     */
    inline Ohlc* writeElt()
    {
        Ohlc *res = nullptr;

        if (full()) {
            res = m_last;
            increment(m_last);
            m_first = m_last;
        } else {
            res = m_last;
            increment(m_last);
            ++m_size;
        }

        return res;
    }

    inline Cit cbegin() const { return Cit(this, m_first); }
    inline Cit cend() const { return Cit(this, nullptr); }

    inline o3d::Int32 size() const { return m_size; }
    inline o3d::Bool full() const { return m_size == getSize(); }

private:

    Ohlc* m_first;
    Ohlc* m_last;
    Ohlc* m_end;
    o3d::Int32 m_size;

    OhlcCircular() : AtomicArray(), m_first(nullptr), m_last(nullptr), m_end(nullptr), m_size(0) {}

    void increment(Ohlc*& p) {
        if (++p == m_end) {
            p = get(0);
        }
    }
};

} // namespace siis

#endif // SIIS_OHLC_H
