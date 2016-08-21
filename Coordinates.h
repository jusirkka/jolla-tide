// $Id: Coordinates.hh 4327 2012-02-18 22:55:54Z flaterco $

/*  Coordinates:  Degrees latitude and longitude.

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

#ifndef COORDINATES_H
#define COORDINATES_H

#include <QString>


namespace Tide {
class Coordinates {

public:

    static Coordinates fromWGS84LatLong(double lat, double lng);

    Coordinates(const Coordinates& a): m_Latitude(a.m_Latitude), m_Longitude(a.m_Longitude), m_Datum(a.m_Datum) {}
    Coordinates& operator=(const Coordinates& a) {m_Latitude = a.m_Latitude; m_Longitude = a.m_Longitude; m_Datum = a.m_Datum; return *this;}
    Coordinates(): m_Latitude(0), m_Longitude(0), m_Datum("Invalid") {}

    double latitude() const {return m_Latitude;}
    double longitude() const {return m_Longitude;}
    const QString& datum() const {return m_Datum;}


private:

    Coordinates(double lat, double lng, const QString& datum): m_Latitude(lat), m_Longitude(lng), m_Datum(datum) {}

    double m_Latitude;
    double m_Longitude;
    QString m_Datum;
};
}
#endif
