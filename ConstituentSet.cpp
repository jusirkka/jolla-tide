// $Id: ConstituentSet.cc 3491 2009-09-04 21:40:05Z flaterco $
/*
    ConstituentSet:  set of constituents, datum, and related methods.

    Copyright (C) 1998  David Flater.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ConstituentSet.h"


/* tideBlendInterval
 *   Half the number of seconds over which to blend the tides from
 *   one epoch to the next.
 */
static const Interval tideBlendInterval = Interval::fromSeconds(3600);


ConstituentSet::ConstituentSet(const Amplitude& d, const QList<Constituent>& cs):
    m_Datum(d), m_Constituents(cs) {

    Constituent c = m_Constituents.first();
    m_L = c.amplitude.L;
    m_T = c.amplitude.T;

    changeYear(Year::currentYear());
}



const Amplitude& ConstituentSet::tideDerivativeMax(unsigned deriv) {
    if (!m_MaxDt.contains(deriv)) {

        QMap<Year, Amplitude> maxima;
        foreach (Constituent c, m_Constituents) {
            foreach (Year y, c.years()) {
                Amplitude k = Amplitude::pow(c.speed.radiansPerSecond, 0, -1, deriv) * c.amplitude * c.nodeFactor(y);
                if (maxima.contains(y)) {
                    maxima[y] = maxima[y] + k;
                } else {
                    maxima[y] = k;
                }
            }
        }

        Constituent c = m_Constituents.first();
        Year y = c.years().first();
        Amplitude mx = Amplitude::pow(c.speed.radiansPerSecond, 0, -1, deriv) * c.amplitude * c.nodeFactor(y);
        foreach (Amplitude a, maxima.values()) {
            if (a > mx) {
                mx = a;
            }
        }

        m_MaxDt[deriv] = mx * 1.1; // add a little safety margin...
    }
    return m_MaxDt.constFind(deriv).value();
}


bool ConstituentSet::isCurrent() const {
    return m_T < 0;
}

// Update amplitudes, phases, epoch, nextEpoch, and currentYear.
void ConstituentSet::changeYear(const Year& newYear) {

    m_CurrentYear = newYear;
    m_Epoch = Timestamp::fromUTCYear(m_CurrentYear);
    m_NextEpoch = Timestamp::fromUTCYear(m_CurrentYear + 1);

    m_Amplitudes.clear();
    m_Phases.clear();
    foreach (Constituent c, m_Constituents) {
        m_Amplitudes.append(c.amplitude * c.nodeFactor(m_CurrentYear));
        m_Phases.append(c.phase + c.equiliriumArgument(m_CurrentYear));
    }
}


// The following block of functions is slightly revised from the code
// delivered by Geoffrey T. Dairiki for XTide 1.  The commentary has
// been modified to try to keep consistent with the code maintenance,
// but inconsistenties probably remain.

/*************************************************************************
 *
 * Geoffrey T. Dairiki Fri Jul 19 15:44:21 PDT 1996
 *
 ************************************************************************/

/*
 * We will need a function for tidal height as a function of time
 * which is continuous (and has continuous first and second derivatives)
 * for all times.
 *
 * Since the epochs and multipliers for the tidal constituents change
 * with the year, tideDerivative(Interval) has small discontinuities
 * at new year's.  These discontinuities really fry the fast
 * root-finders.
 *
 * We will eliminate the new-year's discontinuities by smoothly
 * interpolating (or "blending") between the tides calculated with one
 * year's coefficients and the tides calculated with the next year's
 * coefficients.
 *
 * i.e. for times near a new year's, we will "blend" a tide as follows:
 *
 * tide(t) = tide(year-1, t)
 *                  + w((t - t0) / Tblend) * (tide(year,t) - tide(year-1,t))
 *
 * Here:  t0 is the time of the nearest new-year.
 *        tide(year-1, t) is the tide calculated using the coefficients
 *           for the year just preceding t0.
 *        tide(year, t) is the tide calculated using the coefficients
 *           for the year which starts at t0.
 *        Tblend is the "blending" time scale.  This is set by
 *           the macro TIDE_BLEND_TIME, currently one hour.
 *        w(x) is the "blending function", whice varies smoothly
 *           from 0, for x < -1 to 1 for x > 1.
 *
 * Derivatives of the blended tide can be evaluated in terms of derivatives
 * of w(x), tide(year-1, t), and tide(year, t).  The blended tide is
 * guaranteed to have as many continuous derivatives as w(x).  */


/* tideDerivative (Interval sinceEpoch, unsigned deriv)
 *
 * Calculate (deriv)th time derivative of the normalized tide for time
 * in s since the beginning (UTC) of currentYear, WITHOUT changing
 * years or blending.
 *
 * Note:  This function does not check for changes in year.  This is
 * important to our algorithm, since for times near new year's, we
 * interpolate between the tides calculated using one year's
 * coefficients and the next year's coefficients.
 */

// DWF 2007-02-15

// XTide spends more time in this method than anywhere else.

// In XTide 2.8.3 and previous, the high-level data types (Speed,
// Amplitude, Interval, etc.) were used to shuttle data around, but at
// the last minute everything reverted to C arrays of doubles just to
// make this loop run faster.  The Great Cleanup of 2006 got rid of
// that hypocrisy.  Most use cases showed no noticeable impact, but
// those that involved generating a really long series of predictions
// (e.g., for stats mode or calendar mode) initially showed alarming
// slowdowns of 300% and worse.  Conversion of select methods and
// functions to inlines, plus the avoidance of one type conversion
// that could not be inlined, shaved the performance hit to about 15%,
// which is close enough to argue that the benefits of high-level data
// types exceed the costs.

Amplitude ConstituentSet::tideDerivative(const Interval& sinceEpoch, unsigned deriv) const {
    Amplitude dt_tide = Amplitude::pow(0, m_L, m_T - deriv, 1); // set correct units
    Angle dt_phase = Angle::fromRadians(M_PI / 2.0 * deriv);

    for (int i = 0; i < m_Amplitudes.length(); i++) {
        Angle arg = dt_phase + m_Constituents[i].speed * sinceEpoch + m_Phases[i];
        dt_tide += Amplitude::pow(m_Constituents[i].speed.radiansPerSecond, 0, -1, deriv) * m_Amplitudes[i] * arg.cos();
    }
    return dt_tide;
}


/* blendWeight (double x, unsigned deriv)
 *
 * Returns the value (deriv)th derivative of the "blending function" w(x):
 *
 *   w(x) =  0,     for x <= -1
 *
 *   w(x) =  1/2 + (15/16) x - (5/8) x^3 + (3/16) x^5,
 *                  for  -1 < x < 1
 *
 *   w(x) =  1,     for x >= 1
 *
 * This function has the following desirable properties:
 *
 *    w(x) is exactly either 0 or 1 for |x| > 1
 *
 *    w(x), as well as its first two derivatives are continuous for all x.
 */

static double blendWeight(double x, unsigned deriv) {


    static double x15over16 = 15.0/16;
    static double x15over4 = 15.0/4;
    static double x45over2 = 45.0/2;

    double x2 = x * x;

    if (x2 >= 1.0) {
        if (deriv == 0 && x > 0) {
            return 1.0;
        }
        return 0.0;
    }

    switch (deriv) {
    case 0: return ((3.0 * x2 - 10.0) * x2 + 15.0) * x / 16.0 + 0.5;
    case 1: return ((x2 - 2.0) * x2 + 1.0) * x15over16;
    case 2: return (x2 - 1.0) * x * x15over4;
    case 3: return (3*x2 - 1.0) * x15over4;
    case 4: return x * x45over2;
    case 5: return x45over2;
    }
    return 0.0;
}


/*
 * This function does the actual "blending" of the tide and its
 * derivatives.
 */

Amplitude ConstituentSet::blendTide(const Timestamp& predictTime, unsigned deriv, const Year& firstYear, double blend) {

    QMap<unsigned, Amplitude> firstYearDts;
    QMap<unsigned, Amplitude> secondYearDts;
    QMap<unsigned, Amplitude> blends;


    changeYear(firstYear);
    Interval sinceEpoch = predictTime - m_Epoch;
    for (unsigned n = 0; n <= deriv; ++n) {
        firstYearDts[n] = tideDerivative(sinceEpoch, n);
        blends[n] = blendWeight(blend, n) * Amplitude::pow(tideBlendInterval.seconds, 0, 1, -n);
    }
    changeYear(firstYear + 1);
    sinceEpoch = predictTime - m_Epoch;
    for (unsigned n = 0; n <= deriv; ++n) {
        secondYearDts[n] = tideDerivative(sinceEpoch, n);
    }


    // Do the blending.
    double fact = 1.0;
    Amplitude f = firstYearDts[deriv];
    // Leibniz rule
    for (unsigned n = 0; n <= deriv; ++n) {
        f += fact * blends[n] * (secondYearDts[deriv-n] - firstYearDts[deriv-n]);
        fact *=  ((double)(deriv - n)) / (n + 1); // binomial factor
    }
    return f;
}


// Calculate (deriv)th time derivative of the normalized tide (for
// time in s).  The result does not have the datum added in and will
// not be converted from KnotsSquared.

Amplitude ConstituentSet::tideDerivative(const Timestamp& predictTime, unsigned deriv) {

    // For starters, get us in the right year.
    Year year = predictTime.year();
    if (year != m_CurrentYear) {
        changeYear(year);
    }

    Interval sinceEpoch = predictTime - m_Epoch;


    if (sinceEpoch <= tideBlendInterval) { // after new year
        return blendTide(predictTime, deriv, m_CurrentYear - 1, sinceEpoch / tideBlendInterval);
    }

    Interval tillNextEpoch = m_NextEpoch - predictTime;
    if (tillNextEpoch <= tideBlendInterval) {
        return blendTide(predictTime, deriv, m_CurrentYear, - (tillNextEpoch / tideBlendInterval));
    }

    //  Else, we're far enough from newyear's to ignore the blending.
    return tideDerivative(sinceEpoch, deriv);
}


