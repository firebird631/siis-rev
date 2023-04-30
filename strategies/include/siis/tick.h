/**
 * @brief SiiS strategy bid/ask tick model and array.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#ifndef SIIS_TICK_H
#define SIIS_TICK_H

#include "base.h"

#include <o3d/core/atomicarray.h>
#include <o3d/core/base.h>
#include <o3d/core/memorydbg.h>
#include <o3d/core/memorymanager.h>

using o3d::MemoryManager;

namespace siis {

/**
 * @brief SiiS strategy price bid/ask model and array
 * @author Frederic Scherma
 * @date 2019-03-07
 * It contain 6 double values arranged : timestamp, bid, ask, last, volume, buy/sell.
 * Plus 2 ignored double for align.
 */
class SIIS_API Tick
{
public:

    //! Atomic array initialization and set to identity
    inline void _initAtomicObject(void *ptr)
    {
        D = reinterpret_cast<o3d::Double*>(ptr);
        zero();
    }

    inline void zero()
    {
        memset(D, 0, 6*8); //6*sizeof(o3d::Double));
    }

    Tick()
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(64));
        zero();
    }

    Tick(o3d::Double ts, o3d::Double bid, o3d::Double ask, o3d::Double last, o3d::Double v, o3d::Int8 bos)
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(64));
        D[0] = ts;
        D[1] = bid;
        D[2] = ask;
        D[3] = last;
        D[4] = v;
        D[5] = static_cast<o3d::Double>(bos);
    }

    Tick(const Tick &d)
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(64));
        memcpy(D, d.D, 6*sizeof(o3d::Double));
    }

    ~Tick() { O3D_FAST_FREE(D, 64); }

    inline void copy(const o3d::Double *d)
    {
        memcpy(D, d, 6*sizeof(o3d::Double));
    }

    inline void set(o3d::Double ts, o3d::Double bid, o3d::Double ask, o3d::Double last, o3d::Double vol, o3d::Int8 bos)
    {
        D[0] = ts;
        D[1] = bid;
        D[2] = ask;
        D[3] = last;
        D[4] = vol;
        D[5] = static_cast<o3d::Double>(bos);
    }

    inline o3d::Double timestamp() const { return D[0]; }
    inline o3d::Double ts() const { return D[0]; }

    inline o3d::Double bid() const { return D[1]; }
    inline o3d::Double b() const { return D[1]; }

    inline o3d::Double ofr() const { return D[2]; }
    inline o3d::Double ask() const { return D[2]; }
    inline o3d::Double o() const { return D[2]; }
    inline o3d::Double a() const { return D[2]; }

    inline o3d::Double last() const { return D[3]; }
    inline o3d::Double l() const { return D[3]; }

    inline o3d::Double volume() const { return D[4]; }
    inline o3d::Double vol() const { return D[4]; }
    inline o3d::Double v() const { return D[4]; }

    inline o3d::Int8 buyOrSell() const { return static_cast<o3d::Int8>(D[5]); }

    inline o3d::Double price() const { return (D[1]+D[2])*0.5; }
    inline o3d::Double p() const { return (D[1]+D[2])*0.5; }

    inline o3d::Double spread() const { return D[2] - D[1]; }

    inline o3d::String toString() const
    {
        return o3d::String("ts={0}, bid={1}, ask={2}, last={3}, vol={4}, bos={5}").arg(D[0]).arg(D[1]).arg(D[2]).
                arg(D[3]).arg(D[4]).arg(static_cast<o3d::Int8>(D[5]));
    }

private:

    o3d::Double *D;

    //! Initialization constructor from an external double*
    //! @note _D must be 16 bytes aligned
    inline Tick(o3d::Double *_D) { D = _D; }
};

/**
 * @brief TickArray Tick 16 bytes memory aligned array for data, with object management and a growth capacity.
 * It is 64 bytes size and can contains 8 doubles but only 6 are payload.
 */
class SIIS_API TickArray : public o3d::AtomicArray<Tick, o3d::Double, 6>
{
public:

    //! default constructor (no elements)
    TickArray() : AtomicArray() {}

    /**
     * @brief OhlcArray Initialization constructor
     * @param NbElt Number of element to allocate.
     */
    TickArray(o3d::Int32 nbElt) : AtomicArray(nbElt)
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
            size_t nbytes = static_cast<size_t>(getSize()*8*8);
            o3d::Double *cpy = new o3d::Double[static_cast<size_t>(getSize()*8)];
            memcpy(cpy, getData(), nbytes);  // its linear

            o3d::Int32 prevSize = getSize();
            o3d::Int32 newsize = getMaxSize() * 2;
            resize(newsize);

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
    inline void push(const Tick &tick)
    {
        o3d::Int32 t = getSize();

        // grow output size
        if (getSize() > 0 && t >= getMaxSize()-1) {
            growSize();
        }

        *get(t) = tick;
    }

    inline Tick last() const
    {
        if (getSize() > 0) {
            return *get(getSize()-1);
        }

        return Tick();
    }
};

} // namespace siis

#endif // SIIS_TICK_H
