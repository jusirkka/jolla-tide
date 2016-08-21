// $Id: Interval.hh 2641 2007-09-02 21:31:02Z flaterco $

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

#ifndef INTERVAL_H
#define INTERVAL_H

#include <QtGlobal>


namespace Tide {

// There is no overflow checking on Intervals.

class Timestamp;

typedef qint64 interval_rep_t;

class Interval {
public:

    friend Interval operator- (const Timestamp& a, const Timestamp& b);
    friend Interval operator+ (const Interval& a, const Interval& b);
    friend Interval operator* (int a, const Interval& b);

    interval_rep_t seconds;

    static Interval fromSeconds(interval_rep_t);

    Interval operator- () const; // unary minus

    Interval(const Interval& a): seconds(a.seconds) {}
    Interval& operator=(const Interval& a) {seconds = a.seconds; return *this;}
    Interval(): seconds(0) {}


private:

    Interval(interval_rep_t s): seconds(s) {}

};

Interval operator- (const Timestamp& a, const Timestamp& b);
Interval operator+ (const Interval& a, const Interval& b);
Interval operator* (int a, const Interval& b);
double operator/ (const Interval& a, const Interval& b);

bool operator<= (const Interval& a, const Interval& b);
bool operator< (const Interval& a, const Interval& b);
bool operator> (const Interval& a, const Interval& b);
bool operator== (const Interval& a, const Interval& b);

}
#endif
