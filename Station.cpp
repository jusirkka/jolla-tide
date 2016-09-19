// $Id: Station.cc 5034 2013-07-08 01:58:13Z flaterco $

/*  Station  A tide station.

    Station has a subclass SubordinateStation.  The superclass is used
    for reference stations and that rare subordinate station where the
    offsets can be reduced to simple corrections to the constituents
    and datum.  After such corrections are made, there is no
    operational difference between that and a reference station.

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

#include "Station.h"
#include <cmath>
#include <QDebug>

using namespace Tide;

Station::Station(ConstituentSet* constituents,
                 const QString& aName,
                 const Coordinates& sCoordinates):
    m_Constituents(constituents),
    m_EventPrecision(Interval::fromSeconds(15)),
    m_Name(aName),
    m_Coordinates(sCoordinates),
    m_TZ_name(""),
    m_MinCurrentBearing(),
    m_MaxCurrentBearing(),
    m_Note("")
{}


Station::~Station() {
    delete m_Constituents;
}


Amplitude Station::predictTideLevel(const Timestamp& predictTime) const {
    if (!isvalid()) return Amplitude();
    return m_Constituents->datum() + m_Constituents->tideDerivative(predictTime, 0);
}




void Station::predictTideEvents(const Timestamp& startTime,
                                const Timestamp& endTime,
                                TideEvent::Organizer& organizer,
                                const Amplitude& mark,
                                TideEventsFilter filter) const {

    if (startTime >= endTime) {
        addInvalid(organizer, startTime);
        return;
    }

    if (!isvalid()) {
        addInvalid(organizer, endTime);
        return;
    }

    Timestamp ev_Time;
    TideEvent::Type ev_Type;
    bool isRising;
    Amplitude zero = m_Constituents->datum() * 0; // zero level with correct units



    Timestamp loopTime = startTime;
    while (loopTime <= endTime) {
        Timestamp previousLoopTime = loopTime;

        // Get next max or min.
        ev_Type = nextMaxMin(loopTime, ev_Time);
        loopTime = ev_Time;
        if (ev_Time >= startTime && ev_Time < endTime) {
            addToOrganizer(organizer, ev_Type, ev_Time);
        }

        // Check for slacks, if applicable.
        if (filter != maxMin && isCurrent()) {
            ev_Time = findMarkCrossing(previousLoopTime, loopTime, zero, isRising);
            if (ev_Time >= previousLoopTime && ev_Time <= loopTime) {
                ev_Type = isRising ? TideEvent::slackrise : TideEvent::slackfall;
                addToOrganizer(organizer, ev_Type, ev_Time);
            }
        }

        // Check for mark, if applicable.
        if (markSet(mark) && filter == noFilter) {
            ev_Time = findMarkCrossing(previousLoopTime, loopTime, mark, isRising);
            if (ev_Time >= previousLoopTime && ev_Time <= loopTime) {
                ev_Type = isRising ? TideEvent::markrise : TideEvent::markfall;
                addToOrganizer(organizer, ev_Type, ev_Time);
            }
        }
    }
}


// Analogous to predictTideEvents for raw readings.
void Station::predictRawEvents(const Timestamp& startTime, const Timestamp& endTime, const Interval& step,
                               TideEvent::Organizer& organizer) const {
    if (startTime >= endTime || !isvalid()) {
        return;
    }

    for (Timestamp t = startTime; t < endTime; t += step) {
        addToOrganizer(organizer, TideEvent::rawreading, t);
    }
}

void Station::extendRange(TideEvent::Organizer& organizer,
                          const Interval& range,
                          const Amplitude& mark,
                          TideEventsFilter filter) const {

    Interval zero = Interval();
    if (range == zero || !isvalid()) {
        return;
    }

    Timestamp startTime;
    Timestamp endTime;
    if (range < zero) {
        endTime = organizer.constBegin().value().time;
        startTime = endTime + range;
    } else {
        startTime = organizer.constEnd().value().time;
        endTime = startTime + range;
    }
    predictTideEvents(startTime, endTime, organizer, mark, filter);
}

void Station::extendRange(TideEvent::Organizer& organizer, const Interval& delta, int steps) const {

    if (steps == 0 || !isvalid()) {
        return;
    }

    Timestamp startTime;
    Timestamp endTime;
    if (steps < 0) {
        endTime = organizer.constBegin().value().time;
        startTime = endTime + steps * delta;
    } else {
        startTime = organizer.constEnd().value().time + delta;
        endTime = startTime + steps * delta;
    }
    predictRawEvents(startTime, endTime, delta, organizer);

}




// The following block of methods is slightly revised from the code
// delivered by Geoffrey T. Dairiki for XTide 1.  Jeff's original
// comments (modulo a few global replacements) are shown in C-style,
// while mine are in C++ style.  As usual, see also Station.hh.

/*************************************************************************
 *
 * Geoffrey T. Dairiki Fri Jul 19 15:44:21 PDT 1996
 *
 ************************************************************************/



/*
 *   We are guaranteed to find all high and low tides as long as their
 *   spacing is greater than Global::eventPrecision.
 */



Amplitude Station::MaxMinZeroFn::get(const Timestamp& t, unsigned deriv) const {
    return m_Parent->m_Constituents->tideDerivative (t, deriv + 1);
}


Amplitude Station::MarkZeroFn::get(const Timestamp& t, unsigned deriv) const {
    Amplitude td = m_Parent->m_Constituents->tideDerivative (t, deriv);
    if (deriv == 0) {
        td -= m_Marklevel;
    }
    return td;
}



/* findZero (time_t t1, time_t t2, double (*f)(time_t t, int deriv))
 *   Find a zero of the function f, which is bracketed by t1 and t2.
 *   Returns a value which is either an exact zero of f, or slightly
 *   past the zero of f.
 */

/*
 * Here's a root finder based upon a modified Newton-Raphson method.
 */

Timestamp Station::findZero(const Timestamp& t_left, const Timestamp& t_right, const TestFunc& func) const {

    if (t_left >= t_right) {
        return Timestamp();
    }

    Timestamp tl = t_left;
    Timestamp tr = t_right;

    Amplitude fl = func.get(tl, 0);
    Amplitude fr = func.get(tr, 0);

    double scale = fl.value > 0 ? -1.0 : 1.0;

    fl *= scale;
    fr *= scale;

    if (fl.value >= 0.0 || fr.value <= 0.0) {
        return Timestamp();
    }

    Timestamp t;
    Amplitude ft;

    Amplitude fp;
    Amplitude f_thresh;

    Interval dt;

    static const Interval zero;

    while (tr - tl > m_EventPrecision) {
        if (t.posix() == 0) {
            dt = zero; // First step is bisection
        } else if (std::abs(ft.value) > f_thresh.value // not decreasing fast enough */
                   || (ft.value > 0.0 ? // newton step would go outside bracket
                       (fp.value <= ft.value / (t - tl).seconds) :
                       (fp.value <= -ft.value / (tr - t).seconds))) {
            dt = zero; // Force is bisection
        } else {
            // Attempt a newton step
            if (fp.value == 0.0) {
                return Timestamp();
            }

            // Here I actually do want to round away from zero.
            dt = Interval::fromSeconds(llround(-ft.value/fp.value));

            // Since our goal specifically is to reduce our bracket size as quickly as possible (rather than getting as close to
            // the zero as possible) we should ensure that we don't take steps which are too small. (We'd much rather step over
            // the root than take a series of steps that approach the root rapidly but from only one side.)
            if (std::abs(dt.seconds) < m_EventPrecision.seconds) {
                dt = (ft.value < 0.0 ? m_EventPrecision : - m_EventPrecision);
            }

            t += dt;
            if (t >= tr || t <= tl) {
                dt = Interval();  // Force bisection if outside bracket
            }

            f_thresh = ft.value > 0 ? 0.5 * ft : -0.5 * ft;
        }

        if (dt == zero) {
            // Newton step failed, do bisection
            t = tl + 0.5 * (tr - tl);
            f_thresh = fr > -fl ? fr : -fl;
        }

        ft = scale * func.get(t, 0);
        if (ft.value == 0.0) {
            return t; // Exact zero
        }

        if (ft.value > 0.0) {
            tr = t;
            fr = ft;
        } else {
            tl = t;
            fl = ft;
        }
        fp = scale * func.get(t, 1);
    }

    return tr;
}


/* next_zero(time_t t, double (*f)(), double max_fp, double max_fpp)
 *   Find the next zero of the function f which occurs after time t.
 *   The arguments max_fp and max_fpp give the maximum possible magnitudes
 *   that the first and second derivative of f can achieve.
 *
 *   Algorithm:  Our goal here is to bracket the next zero of f ---
 *     then we can use findZero() to quickly refine the root.
 *     So, we will step forward in time until the sign of f changes,
 *     at which point we know we have bracketed a root.
 *     The trick is to use large steps in our search, making
 *     sure the steps are not so large that we inadvertently
 *     step over more than one root.
 *
 *     The big trick, is that since the tides (and derivatives of
 *     the tides) are all just harmonic series's, it is easy to place
 *     absolute bounds on their values.
 */

// This method is only used in one place and is only used for finding
// maxima and minima, so I renamed it to nextMaxMin, got rid of the
// max_fp and max_fpp parameters, and installed a more convenient out
// parameter.

// Since by definition the tide cannot change direction between maxima
// and minima, there is at most one crossing of a given mark level
// between min/max points.  Therefore, we already have a bracket of
// the mark level to give to findZero, and there is no need for a
// function like this to find the next mark crossing.

TideEvent::Type Station::nextMaxMin(const Timestamp& t, Timestamp &eventTime_out) const {

    static const Amplitude max_fp = m_Constituents->tideDerivativeMax(2);
    static const Amplitude max_fpp = m_Constituents->tideDerivativeMax(3);

    Timestamp t_left, t_right;
    Interval step, step1, step2;
    Amplitude f_left, df_left, f_right;
    double scale = 1.0;

    MaxMinZeroFn func(this);
    // If we start at a zero, step forward until we're past it.
    for (t_left = t, f_left = func.get(t_left, 0);
         f_left.value == 0.0;
         t_left += m_EventPrecision, f_left = func.get(t_left, 0)) {
        // empty
    }

    TideEvent::Type ret;
    if (f_left.value < 0.0) {
        ret = TideEvent::min;
    } else {
        ret = TideEvent::max;
        scale = -1.0;
        f_left = -f_left;
    }

    while (true) {

        // Minimum time to next zero
        step1 = Interval::fromSeconds((interval_rep_t)(std::abs(f_left.value) / max_fp.value));

        // Minimum time to next turning point
        df_left = scale * func.get(t_left, 1);
        step2 = Interval::fromSeconds((interval_rep_t)(std::abs(df_left.value) / max_fpp.value));

        if (df_left.value < 0.0) {
            // Derivative is in the wrong direction
            step = step1 + step2;
        } else {
            step = step1 > step2 ? step1 : step2;
        }

        if (step < m_EventPrecision) {
            step = m_EventPrecision; // No ridiculously small steps
        }

        t_right = t_left + step;

        // If we hit upon an exact zero, step right until we're off the zero. If the sign has changed, we are bracketing a desired
        // root.  If the sign hasn't changed, then the zero was at an inflection point (i.e. a double-zero to within m_EventPrecision)
        // and we want to ignore it.

        for (t_right = t_left + step, f_right = scale * func.get(t_right, 0);
             f_right.value == 0.0;
             t_right += m_EventPrecision, f_right = func.get(t_right, 0)) {
            // empty
        }


        if (f_right.value > 0.0) {  /* Found a bracket */
            eventTime_out = findZero(t_left, t_right, func);
            return ret;
        }

        t_left = t_right;
        f_left = f_right;
    }
}


Timestamp Station::findMarkCrossing_Dairiki(const Timestamp& t_left,
                                            const Timestamp& t_right,
                                            const MarkZeroFn& func,
                                            bool &isRising_out) const {

    Timestamp t1 = t_left;
    Timestamp t2 = t_right;

    if (t_left > t_right) {
        t1 = t_right;
        t2 = t_left;
    }


    Amplitude f1 = func.get(t1, 0);
    Amplitude f2 = func.get(t2, 0);

    // Fail gently on rotten brackets.  (This used to be an assertion.)
    if (f1 == f2) {
        return Timestamp(); // return null timestamp
    }

    // We need || instead of && to set isRising_out correctly in the
    // case where there's a zero exactly at t1 or t2.
    isRising_out = f1.value < 0.0 || f2.value > 0.0;
    if (!isRising_out) {
        f1 = -f1;
        f2 = -f2;
    }

    // Since f1 != f2, we can't get two zeros, so it doesn't matter which
    // one we check first.
    if (f1.value == 0.0) {
        return t1;
    }

    if (f2.value == 0.0) {
        return t2;
    }

    if (f1.value < 0.0 && f2.value > 0.0) {
        return findZero(t1, t2, func);
    }

    return Timestamp(); // Don't have a bracket, return null timestamp.
}


/*************************************************************************/


Timestamp Station::findMarkCrossing(const Timestamp& t1,
                                    const Timestamp& t2,
                                    const Amplitude& markLevel,
                                    bool &isRising_out) const {

    // marklev must compensate for datum and KnotsSquared. See markZeroFn.
    // Units should already be comparable to datum.
    MarkZeroFn func(this, markLevel - m_Constituents->datum());

    return findMarkCrossing_Dairiki (t1, t2, func, isRising_out);
}



void Station::addToOrganizer(TideEvent::Organizer& organizer, TideEvent::Type tp, const Timestamp& ts) const {
    TideEvent event;
    event.time = ts;
    event.type = tp;
    event.level = predictTideLevel(ts);
    organizer.insert(ts, event);
}

void Station::addInvalid(TideEvent::Organizer& org, const Timestamp& ts) const {
    TideEvent event;
    event.time = ts;
    event.type = TideEvent::invalid;
    org.insert(ts, event);
}

