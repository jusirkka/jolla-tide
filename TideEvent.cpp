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
    // TODO: qstrid
    switch (type) {
    case TideEvent::max:
        if (isCurrent()) {
            if (level.value >= 0.0) {
                return "Max Flood";
            }
            return "Min Ebb";
        }
        return "High Tide";
    case TideEvent::min:
        if (isCurrent()) {
            if (level.value <= 0.0) {
                return "Max Ebb";
            }
            return "Min Flood";
        }
        return "Low Tide";
    case TideEvent::slackrise:
        return "Slack, Flood Begins";
    case TideEvent::slackfall:
        return "Slack, Ebb Begins";
    case TideEvent::markrise:
        if (isCurrent()) {
            if (level.value < 0.0) {
                return "Mark, Ebb Decreasing";
            }
            if (level.value > 0.0) {
                return "Mark, Flood Increasing";
            }
            return "Mark, Flood Begins";
        }
        return "Mark Rising";
    case TideEvent::markfall:
        if (isCurrent()) {
            if (level.value < 0.0) {
                return "Mark, Ebb Increasing";
            }
            if (level.value > 0.0) {
                return "Mark, Flood Decreasing";
            }
            return "Mark, Ebb Begins";
        }
        return "Mark Falling";
    case TideEvent::sunrise:
        return "Sunrise";
    case TideEvent::sunset:
        return "Sunset";
    case TideEvent::moonrise:
        return "Moonrise";
    case TideEvent::moonset:
        return "Moonset";
    case TideEvent::newmoon:
        return "New Moon";
    case TideEvent::firstquartermoon:
        return "First Quarter Moon";
    case TideEvent::fullmoon:
        return "Full Moon";
    case TideEvent::lastquartermoon:
        return "Last Quarter Moon";
    case TideEvent::rawreading:
    case TideEvent::invalid:
        return "Cannot find any events";
    default:
        return "Raw reading";
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
