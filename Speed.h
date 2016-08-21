// $Id: Speed.hh 2641 2007-09-02 21:31:02Z flaterco $

// Speed:  angular units over time units.

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

#ifndef SPEED_H
#define SPEED_H

namespace Tide {

class Speed {
public:

    double radiansPerSecond;

    static Speed fromRadiansPerSecond(double);
    static Speed fromDegreesPerHour(double);

    Speed(const Speed& a): radiansPerSecond(a.radiansPerSecond) {}
    Speed& operator=(const Speed& a) {radiansPerSecond = a.radiansPerSecond; return *this;}
    Speed(): radiansPerSecond(0) {}


private:


    Speed(double rps): radiansPerSecond(rps) {}


};

}
#endif

