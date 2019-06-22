/**
 * @brief SiiS strategy bid/ofr tick model and array.
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
 * @brief SiiS strategy price bid/ofr model and array
 * @author Frederic Scherma
 * @date 2019-03-07
 * It contain 4 double values arranged : timestamp, bid, ofr, volume.
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
        memset(D, 0, 24); //4*sizeof(o3d::Double));
    }

    Tick()
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(32));
        zero();
    }

    Tick(o3d::Double ts, o3d::Double bid, o3d::Double ofr, o3d::Double v)
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(32));
        D[0] = ts;
        D[1] = bid;
        D[2] = ofr;
        D[3] = v;
    }

    Tick(const Tick &d)
    {
        D = reinterpret_cast<o3d::Double*>(O3D_FAST_ALLOC(32));
        memcpy(D, d.D, 4*sizeof(o3d::Double));
    }

    ~Tick() { O3D_FAST_FREE(D, 32); }

    inline void copy(const o3d::Double *d)
    {
        memcpy(D, d, 4*sizeof(o3d::Double));
    }

    inline void set(o3d::Double ts, o3d::Double bid, o3d::Double ofr, o3d::Double v)
    {
        D[0] = ts;
        D[1] = bid;
        D[2] = ofr;
        D[3] = v;
    }

    inline o3d::Double timestamp() const { return D[0]; }
    inline o3d::Double ts() const { return D[0]; }

    inline o3d::Double bid() const { return D[1]; }
    inline o3d::Double b() const { return D[1]; }

    inline o3d::Double ofr() const { return D[2]; }
    inline o3d::Double ask() const { return D[2]; }
    inline o3d::Double o() const { return D[2]; }
    inline o3d::Double a() const { return D[2]; }

    inline o3d::Double volume() const { return D[3]; }
    inline o3d::Double vol() const { return D[3]; }
    inline o3d::Double v() const { return D[3]; }

    inline o3d::Double price() const { return (D[1]+D[2])*0.5; }
    inline o3d::Double p() const { return (D[1]+D[2])*0.5; }

    inline o3d::Double spread() const { return D[2] - D[1]; }

    inline o3d::String toString() const
    {
        return o3d::String("ts={0}, bid={1}, ofr={2}, vol={3}").arg(D[0]).arg(D[1]).arg(D[2]).arg(D[3]);
    }

private:

    o3d::Double *D;

    //! Initialization constructor from an external double*
    //! @note _D must be 16 bytes aligned
    inline Tick(o3d::Double *_D) { D = _D; }
};

/**
 * @brief TickArray Tick 16 bytes memory aligned array for data, with object management and a growth capacity.
 */
class SIIS_API TickArray : public o3d::AtomicArray<Tick, o3d::Double, 5>
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
            size_t nbytes = static_cast<size_t>(getSize()*32);
            o3d::Double *cpy = new o3d::Double[static_cast<size_t>(getSize()*4)];
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
};

} // namespace siis

#endif // SIIS_TICK_H
