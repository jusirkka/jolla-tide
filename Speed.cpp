// $Id: Speed.cc 2641 2007-09-02 21:31:02Z flaterco $

// Speed:  angular units over time units.

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

#include "Speed.h"
#include <math.h>

using namespace Tide;

Speed Speed::fromRadiansPerSecond(double val) {
    if (val < 0) throw Negative("Only non-negative angular speeds are accepted");
    return Speed(val);
}

Speed Speed::fromDegreesPerHour(double val) {
    if (val < 0) throw Negative("Only non-negative angular speeds are accepted");
    return Speed(val * M_PI / 648000.0);
}

double Speed::dph() const {
    return radiansPerSecond * 648000.0 / M_PI;
}

Speed& Speed::operator+= (const Speed& a) {
    radiansPerSecond += a.radiansPerSecond;
    return *this;
}

bool (Tide::operator==) (const Speed& a, const Speed& b) {
    static const double eps = 0.0000001;
    double den = 0.5 * (a.radiansPerSecond + b.radiansPerSecond);
    if (den == 0) return true; // both nil
    return fabs(a.radiansPerSecond - b.radiansPerSecond) / den < eps;
}

bool (Tide::operator!=) (const Speed& a, const Speed& b) {
    return !(a == b);
}

bool (Tide::operator<) (const Speed& a, const Speed& b) {
    if (a == b) return false;
    return a.radiansPerSecond < b.radiansPerSecond;
}

bool (Tide::operator<=) (const Speed& a, const Speed& b) {
    if (a == b) return true;
    return a.radiansPerSecond < b.radiansPerSecond;
}

bool (Tide::operator>) (const Speed& a, const Speed& b) {
    return !(a <= b);
}

bool (Tide::operator>=) (const Speed& a, const Speed& b) {
    return !(a < b);
}

Speed (Tide::operator*) (double a, const Speed& b) {
    return Speed(a * b.radiansPerSecond);
}

Speed (Tide::operator*) (const Speed& b, double a) {
    return a * b;
}

