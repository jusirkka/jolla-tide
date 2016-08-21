// $Id: Angle.hh 2641 2007-09-02 21:31:02Z flaterco $

// Angle

/*
    Copyright (C) 1997  David Flater.

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

#ifndef ANGLE_H
#define ANGLE_H

#include <math.h>
#include "Speed.h"
#include "Interval.h"

namespace Tide {

class Angle {
public:

    friend Angle operator+ (const Angle& a, const Angle& b);
    friend Angle operator* (const Speed& a, const Interval& b);

    double radians;

    static Angle fromRadians(double);
    static Angle fromDegrees(double);

    Angle(const Angle& a): radians(a.radians) {}
    Angle& operator=(const Angle& a) {radians = a.radians; return *this;}
    Angle(): radians(0) {}

    double cos() const {return ::cos(radians);}

private:

    Angle(double r): radians(r) {}


};

Angle operator+ (const Angle& a, const Angle& b);
Angle operator* (const Speed& a, const Interval& b);

}



#endif
