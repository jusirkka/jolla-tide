// $Id: TideEvent.hh 2641 2007-09-02 21:31:02Z flaterco $

// TideEvent  Generic representation for tide events.

/*
    Copyright (C) 2004  David Flater.

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

#ifndef TIDEEVENT_H
#define TIDEEVENT_H

#include <QString>
#include <QMap>

#include "Timestamp.h"
#include "Amplitude.h"

namespace Tide {

class TideEvent {
public:

    typedef QMultiMap<Timestamp, TideEvent> Organizer;

    // CamelCasing waived here for consistency with sunrise, sunset.
    enum Type {
        invalid,
        max, min, slackrise, slackfall, markrise, markfall,
        sunrise, sunset, moonrise, moonset, newmoon, firstquartermoon, fullmoon, lastquartermoon,
        rawreading
    };

    TideEvent(): time(), type(invalid), level() {}

    Timestamp time;
    Type type;
    Amplitude level;

    QString description() const;
    QString shortname() const;
    bool isSunMoonEvent() const;
    bool isMaxMinEvent() const;
    bool isCurrent() const;

    // This returns true if the description of the event would be Min Flood or
    // Min Ebb.
    bool isMinCurrentEvent () const;

private:

    QString _description() const;

};

}

#endif
