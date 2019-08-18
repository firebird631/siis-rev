/**
 * @brief SiiS tom demark 9 indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_TD9_H
#define SIIS_TD9_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS tom demark 9 indicator.
 * @author Frederic Scherma
 * @date 2019-03-19
 */
class SIIS_API Td9 : public Indicator
{
public:

    // TYPE_TREND
    // CLS_INDEX

    struct CToken
    {
        o3d::Int32 c;
        o3d::Int32 d;
        o3d::Bool p;

        o3d::Double tdst;

        CToken() :
            c(0),
            d(0),
            p(false),
            tdst(0.0)
        {
        }

        CToken(const CToken &dup) :
            c(dup.c),
            d(dup.d),
            p(dup.p),
            tdst(dup.tdst)
        {
        }

        CToken& operator= (const CToken &dup)
        {
            c = dup.c;
            d = dup.d;
            p = dup.p;
            tdst = dup.tdst;

            return *this;
        }

        void reset()
        {
            c = 0;
            d = 0;
            p = false;
            tdst = 0.0;
        }
    };

    struct CDToken
    {
        o3d::Int32 c;
        o3d::Int32 d;
        o3d::Bool q;

        o3d::Double eight;

        CDToken() :
            c(0),
            d(0),
            q(false),
            eight(0.0)
        {
        }

        CDToken(const CDToken &dup) :
            c(dup.c),
            d(dup.d),
            q(dup.q),
            eight(dup.eight)
        {
        }

        CDToken& operator= (const CDToken &dup)
        {
            c = dup.c;
            d = dup.d;
            q = dup.q;
            eight = dup.eight;

            return *this;
        }

        void reset()
        {
            c = 0;
            d = 0;
            q = false;
            eight = 0.0;
        }
    };

    Td9(const o3d::String &name, o3d::Double timeframe);
    Td9(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    const CToken& c() const { return m_c; }
    const CToken& count() const { return m_c; }

    const CDToken& cd() const { return m_cd; }
    const CDToken& countDown() const { return m_cd; }

    const CDToken& acd() const { return m_aggCd; }
    const CDToken& aggCd() const { return m_aggCd; }
    const CDToken& aggressivCountDown() const { return m_aggCd; }

    /**
     * @brief compute Compute a TD9.
     * @param timestamps An array of the timestamp of each of the given prices (high, low, close).
     */
    void compute(o3d::Double timestamp,
                 const DataArray &timestamps,
                 const DataArray &high,
                 const DataArray &low,
                 const DataArray &close);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    CToken m_c;
    CToken m_prevC;

    CDToken m_cd;
    CDToken m_prevCd;

    CDToken m_aggCd;
    CDToken m_prevAggCd;

    o3d::Double m_highLow;

    void td9(o3d::Int32 b, const DataArray &high, const DataArray &low, const DataArray &close);
};

} // namespace siis

#endif // SIIS_TD9_H
