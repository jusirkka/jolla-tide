// $Id: TideEvent.cc 2833 2007-12-01 01:27:02Z flaterco $

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


#include "Amplitude.h"
#include "Timestamp.h"
#include "TideEvent.h"

using namespace Tide;

bool TideEvent::isSunMoonEvent () const {
    switch (type) {
    case TideEvent::sunrise:
    case TideEvent::sunset:
    case TideEvent::moonrise:
    case TideEvent::moonset:
    case TideEvent::newmoon:
    case TideEvent::firstquartermoon:
    case TideEvent::fullmoon:
    case TideEvent::lastquartermoon:
        return true;
    default:
        return false;
    }
}


bool TideEvent::isMaxMinEvent () const {
    switch (type) {
    case TideEvent::max:
    case TideEvent::min:
        return true;
    default:
        return false;
    }
}

bool TideEvent::isCurrent() const {
    return level.T < 0;
}

bool TideEvent::isMinCurrentEvent () const {
    switch (type) {
    case TideEvent::max:
        return (isCurrent() && (level.value < 0.0));
    case TideEvent::min:
        return (isCurrent() && (level.value > 0.0));
    default:
        return false;
    }
}

QString TideEvent::description () const {
    if (level.valid()) return _description() + QString(" @%1").arg(level.print());
    return _description();
}

QString TideEvent::_description () const {
    switch (type) {
    case TideEvent::max:
        if (isCurrent()) {
            if (level.value >= 0.0) {
                //% "Max Flood"
                return qtTrId("tide-event-max-flood");
            }
            //% "Min Ebb"
            return qtTrId("tide-event-min-ebb");
        }
        //% "High Tide"
        return qtTrId("tide-event-high-tide");
    case TideEvent::min:
        if (isCurrent()) {
            if (level.value <= 0.0) {
                //% "Max Ebb"
                return qtTrId("tide-event-max-ebb");
            }
            //% "Min Flood"
            return qtTrId("tide-event-min-flood");
        }
        //% "Low Tide"
        return qtTrId("tide-event-low-tide");
    case TideEvent::slackrise:
        //% "Slack, Flood Begins"
        return qtTrId("tide-event-slack-flodd-begins");
    case TideEvent::slackfall:
        //% "Slack, Ebb Begins"
        return qtTrId("tide-event-slack-ebb-begins");
    case TideEvent::markrise:
        if (isCurrent()) {
            if (level.value < 0.0) {
                //% "Mark, Ebb Decreasing"
                return qtTrId("tide-event-mark-ebb-decreasing");
            }
            if (level.value > 0.0) {
                //% "Mark, Flood Increasing"
                return qtTrId("tide-event-mark-flood-increasing");
            }
            //% "Mark, Flood Begins"
            return qtTrId("tide-event-mark-flood-begins");
        }
        //% "Mark Rising"
        return qtTrId("tide-event-mark-rising");
    case TideEvent::markfall:
        if (isCurrent()) {
            if (level.value < 0.0) {
                //% "Mark, Ebb Increasing"
                return qtTrId("tide-event-mark-ebb-increasing");
            }
            if (level.value > 0.0) {
                //% "Mark, Flood Decreasing"
                return qtTrId("tide-event-mark-flood-decreasing");
            }
            //% "Mark, Ebb Begins"
            return qtTrId("tide-event-mark-ebb-begins");
        }
        //% "Mark Falling"
        return qtTrId("tide-event-mark-falling");
    case TideEvent::sunrise:
        //% "Sunrise"
        return qtTrId("tide-event-sunrise");
    case TideEvent::sunset:
        //% "Sunset"
        return qtTrId("tide-event-sunset");
    case TideEvent::moonrise:
        //% "Moonrise"
        return qtTrId("tide-event-moonrise");
    case TideEvent::moonset:
        //% "Moonset"
        return qtTrId("tide-event-moonset");
    case TideEvent::newmoon:
        //% "New Moon"
        return qtTrId("tide-event-newmoon");
    case TideEvent::firstquartermoon:
        //% "First Quarter Moon"
        return qtTrId("tide-event-first-quarter-moon");
    case TideEvent::fullmoon:
        //% "Full Moon"
        return qtTrId("tide-event-full-moon");
    case TideEvent::lastquartermoon:
        //% "Last Quarter Moon"
        return qtTrId("tide-event-last-quarter-moon");
    case TideEvent::invalid:
        //% "Cannot find any events"
        return qtTrId("tide-event-invalid");
    case TideEvent::rawreading:
    default:
        //% "Raw reading"
        return qtTrId("tide-event-raw-reading");
    }
}


QString TideEvent::shortname () const {
    switch (type) {
    case TideEvent::max:
        if (isCurrent()) {
            if (level.value >= 0.0) {
                return "max-flood";
            }
            return "min-ebb";
        }
        return "high-tide";
    case TideEvent::min:
        if (isCurrent()) {
            if (level.value <= 0.0) {
                return "max-ebb";
            }
            return "min-flood";
        }
        return "low-tide";
    case TideEvent::slackrise:
        return "slack-flood-begins";
    case TideEvent::slackfall:
        return "slack-ebb-begins";
    case TideEvent::markrise:
        if (isCurrent()) {
            if (level.value < 0.0) {
                return "mark-ebb-decreasing";
            }
            if (level.value > 0.0) {
                return "mark-flood-increasing";
            }
            return "mark-flood-begins";
        }
        return "mark-rising";
    case TideEvent::markfall:
        if (isCurrent()) {
            if (level.value < 0.0) {
                return "mark-ebb-increasing";
            }
            if (level.value > 0.0) {
                return "mark-flood-decreasing";
            }
            return "mark-ebb-begins";
        }
        return "mark-falling";
    case TideEvent::sunrise:
        return "sunrise";
    case TideEvent::sunset:
        return "sunset";
    case TideEvent::moonrise:
        return "moonrise";
    case TideEvent::moonset:
        return "moonset";
    case TideEvent::newmoon:
        return "new-moon";
    case TideEvent::firstquartermoon:
        return "first-quarter-moon";
    case TideEvent::fullmoon:
        return "full-moon";
    case TideEvent::lastquartermoon:
        return "last-quarter-moon";
    case TideEvent::invalid:
        return "invalid";
    case TideEvent::rawreading:
    default:
        return "raw-reading";
    }
}
