// $Id: Station.hh 3749 2010-08-29 12:36:38Z flaterco $

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

#ifndef STATION_H
#define STATION_H

#include <QString>


#include "Angle.h"
#include "Coordinates.h"
#include "Amplitude.h"
#include "Interval.h"
#include "Timestamp.h"
#include "ConstituentSet.h"
#include "TideEvent.h"

namespace Tide {
class Station {
public:

    // Properties
    const QString& name() const {return m_Name;}
    const Coordinates& coordinates() const {return m_Coordinates;}
    const QString& tZName() const {return m_TZ_name;} // Empty if N/A
    const Angle& minCurrentBearing() const {return m_MinCurrentBearing;} // zero if N/A
    const Angle& maxCurrentBearing() const {return m_MaxCurrentBearing;} // zero if N/A
    const QString& note() const {return m_Note;} // Empty if N/A



    Station(const QString& name, const Coordinates& coordinates, const ConstituentSet* constituents);

    virtual ~Station();


    bool isCurrent() const {return m_Constituents->isCurrent();}

    // Get heights or velocities.
    Amplitude predictTideLevel(const Timestamp& predictTime) const;


    // Filters for predictTideEvents.
    // noFilter = maxes, mins, slacks, mark crossings
    // knownTideEvents = tide events that can be determined without interpolation
    //                   (maxes, mins, and sometimes slacks)
    // maxMin = maxes and mins
    enum TideEventsFilter {noFilter, knownTideEvents, maxMin};

    // Get all tide events within a range of timestamps and add them to
    // the organizer.  The range is >= startTime and < endTime.  Because
    // predictions are done to plus or minus one minute, invoking this
    // multiple times with adjoining ranges could duplicate or delete
    // tide events falling right on the boundary.  TideEventsOrganizer
    // should suppress the duplicates, but omissions will not be
    // detected.
    //
    void predictTideEvents(const Timestamp& startTime,
                           const Timestamp& endTime,
                           TideEvent::Organizer& organizer,
                           const Amplitude& mark = Amplitude(),
                           TideEventsFilter filter = noFilter
                           ) const;

    // Analogous, for raw readings.
    void predictRawEvents(const Timestamp& startTime,
                          const Timestamp& endTime,
                          const Interval& step,
                          TideEvent::Organizer &organizer) const;

    // Add events to an organizer to extend its range in the specified
    // direction by the specified interval.  (Number of events is
    // indeterminate.)  A safety margin is used to attempt to prevent
    // tide events from falling through the cracks as discussed above
    // predictTideEvents.
    //
    // Either settings or the filter arg can suppress sun and moon events.
    void extendRange(TideEvent::Organizer& organizer,
                     const Interval& range,
                     const Amplitude& mark = Amplitude(),
                     TideEventsFilter filter = noFilter) const;

    // Analogous, for raw readings.  Specify number of events in howmany.
    void extendRange(TideEvent::Organizer& organizer, const Interval& delta, int steps) const;


    bool markSet(const Amplitude& a) const {return m_Constituents->markSet(a);}


protected:

    const ConstituentSet* m_Constituents;


    // G. Dairiki code, slightly revised.  See Station.cc for
    // more documentation.

    // Functions to zero out.
    class TestFunc {
    public:
        virtual Amplitude get(const Timestamp& t, unsigned deriv) const = 0;
    protected:
        TestFunc(const Station* p): m_Parent(p) {}
        const Station* m_Parent;
    };

    // Option #1 -- find maxima and minima.
    class MaxMinZeroFn: public TestFunc {
    public:
        MaxMinZeroFn(const Station* p): TestFunc(p) {}
        Amplitude get(const Timestamp& t, unsigned deriv) const;
    };

    // Option #2 -- find mark crossings or slack water.
    // ** Marklev must be made compatible with the tide as returned by
    // tideDerivative, i.e., no datum, no conversion from KnotsSquared.
    class MarkZeroFn: public TestFunc {
    public:
        MarkZeroFn(const Station* p, const Amplitude& marklevel): TestFunc(p), m_Marklevel(marklevel) {}
        Amplitude get(const Timestamp& t, unsigned deriv) const;
    private:
        const Amplitude& m_Marklevel;
    };

    friend class MaxMinZeroFn;
    friend class MarkZeroFn;

    // Root finder.
    //   * If tl >= tr, assertion failure.
    //   * If tl and tr do not bracket a root, assertion failure.
    //   * If a root exists exactly at tl or tr, assertion failure.
    Timestamp findZero(const Timestamp& tl, const Timestamp& tr, const TestFunc&) const;

    // Find the marklev crossing in this bracket.  Used for both
    // markLevel and slacks.
    //   * Doesn't matter which of t1 and t2 is greater.
    //   * If t1 == t2, returns null.
    //   * If t1 and t2 do not bracket a mark crossing, returns null.
    //   * If mark crossing is exactly at t1 or t2, returns that.
    Timestamp findMarkCrossing_Dairiki(const Timestamp& t1,
                                       const Timestamp& t2,
                                       const MarkZeroFn& markZeroFn,
                                       bool& isRising_out) const;

    // Find the next maximum or minimum.
    // eventTime and eventType are set to the next event (uncorrected time).
    TideEvent::Type nextMaxMin(const Timestamp& t, Timestamp& eventTime_out) const;


    // Wrapper for findMarkCrossing_Dairiki that does necessary
    // compensations for datum, KnotsSquared, and units.  Used for both
    // markLevel and slacks.
    Timestamp findMarkCrossing(const Timestamp& t1,
                               const Timestamp& t2,
                               const Amplitude& markLevel,
                               bool& isRising_out) const;

    void addToOrganizer(TideEvent::Organizer&, TideEvent::Type, const Timestamp&) const;

protected:

    Interval m_EventPrecision;

private:

    QString m_Name;
    Coordinates m_Coordinates;
    QString m_TZ_name;
    Angle m_MinCurrentBearing;
    Angle m_MaxCurrentBearing;
    QString m_Note;


};

}

#endif
