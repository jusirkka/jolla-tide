// $Id: Interval.cc 2641 2007-09-02 21:31:02Z flaterco $

// Interval:  what you get if you subtract two timestamps.

/*
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

#include "Interval.h"
#include "Timestamp.h"

using namespace Tide;

Interval Interval::fromSeconds(interval_rep_t s) {
    return Interval(s);
}


Interval Interval::operator- () const {
    return Interval(- seconds);
}

Interval (Tide::operator-) (const Timestamp& a, const Timestamp& b) {
    return Interval(a.posix() - b.posix());
}

Interval (Tide::operator+) (const Interval& a, const Interval& b) {
    return Interval(a.seconds + b.seconds);
}

Interval (Tide::operator*) (int a, const Interval& b) {
    return Interval(a * b.seconds);
}

double (Tide::operator/) (const Interval& a, const Interval& b) {
    return ((double) a.seconds) / b.seconds;
}


bool (Tide::operator<=) (const Interval& a, const Interval& b) {
    return a.seconds <= b.seconds;
}

bool (Tide::operator<) (const Interval& a, const Interval& b) {
    return a.seconds < b.seconds;
}

bool (Tide::operator>) (const Interval& a, const Interval& b) {
    return a.seconds > b.seconds;
}

bool (Tide::operator==) (const Interval& a, const Interval& b) {
    return a.seconds == b.seconds;
}

