/**
 * @brief SiiS 16 bytes aligned double circluar data array for manipulation.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-18
 */

#ifndef SIIS_DATACIRCULAR_H
#define SIIS_DATACIRCULAR_H

#include "dataarray.h"

namespace siis {

/**
 * @brief DataCircular Circular array of double.
 */
class SIIS_API DataCircular : private DataArray
{
public:

    struct Cit
    {
        Cit(const DataCircular *b, const o3d::Double *p) : b(b), p(p) {}

        o3d::Bool operator!= (const Cit &_which) const
        {
            return  p != _which.p;
        }

        Cit& operator++ ()
        {
            if (!b->full() && p == b->m_last) {
                p = nullptr;
                return *this;
            }

            ++p;

            if (p == b->m_last) {
                p = nullptr;
                return *this;
            }

            if (p == b->m_end) {
                p = &b->get(0);

                if (p == b->m_last) {
                    p = nullptr;
                }
            }

            return *this;
        }

        const o3d::Double& operator* () const
        {
            return *p;
        }

        const DataCircular *b;
        const o3d::Double *p;
    };

    /**
     * @brief DataCircular Initialization constructor
     * @param size Number of element to allocate.
     */
    DataCircular(o3d::Int32 size) :
        DataArray(size),
        m_first(&get(0)),
        m_last(&get(0)),
        m_end(&get(0)+size),
        m_size(0)
    {
        O3D_ASSERT(size > 1);  // at least 2 elements
    }

    /**
     * @brief clear Set to 0 element but keep the array allocation.
     */
    inline void clear()
    {
        m_first = m_last = &get(0);
        m_size = 0;
        *m_first = 0.0;
    }

    void setSize(o3d::Int32 newSize)
    {
        O3D_ASSERT(newSize > 1);  // at least 2 elements

        if (newSize != getSize()) {
            DataArray::setSize(newSize);
            m_first = &get(0);
            m_last = &get(0);
            m_end = &get(0) + newSize;
            m_size = 0;
        }
    }

    inline o3d::Int32 capacity() const { return getMaxSize(); }

    /**
     * @brief writeElt Return the current writable double and move to the next position.
     * @return A valid pointer on the last double for write.
     */
    inline void append(o3d::Double v)
    {
        o3d::Double *res = nullptr;

        if (full()) {
            res = m_last;
            increment(m_last);
            m_first = m_last;
        } else {
            res = m_last;
            increment(m_last);
            ++m_size;
        }

        *res = v;
    }

    inline Cit cbegin() const { return Cit(this, m_size > 0 ? m_first : nullptr); }
    inline Cit cend() const { return Cit(this, nullptr); }

    inline o3d::Double& front() { return *m_first; }
    inline const o3d::Double& front() const { return *m_first; }

    inline o3d::Double& back() {
        if (m_last > m_first) {
            return *(m_last - 1);
        } if (m_last > m_data) {
            return *(m_last - 1);
        } else {
            return *(m_data+m_size-1);
        }
    }
    inline const o3d::Double& back() const {
        if (m_last > m_first) {
            return *(m_last - 1);
        } if (m_last > m_data) {
            return *(m_last - 1);
        } else {
            return *(m_data+m_size-1);
        }
    }

    inline o3d::Int32 size() const { return m_size; }
    inline o3d::Bool full() const { return m_size == getSize(); }

    /**
     * Convert a circular array into a linear array in one or two memcpy.
     */
    void asArray(DataArray &out) const
    {
        if (out.getSize() != size()) {
            out.setSize(size());
        }

        if (m_last > m_first) {
            // same effect as last case
            memcpy(out.getData(), m_first, m_size * sizeof(o3d::Double));
        } else if (m_last > m_data) {
            o3d::Int32 ofs = m_end - m_first;
            memcpy(out.getData(), m_first, ofs * sizeof(o3d::Double));
            memcpy(out.getData() + ofs, m_data, (m_size - ofs) * sizeof(o3d::Double));
        } else {
            memcpy(out.getData(), m_data, m_size * sizeof(o3d::Double));
        }
    }

private:

    o3d::Double* m_first;
    o3d::Double* m_last;
    o3d::Double* m_end;
    o3d::Int32 m_size;

    DataCircular() : DataArray(), m_first(nullptr), m_last(nullptr), m_end(nullptr), m_size(0) {}

    void increment(o3d::Double*& p) {
        if (++p == m_end) {
            p = &get(0);
        }
    }
};

} // namespace siis

#endif // SIIS_DATACIRCULAR_H
