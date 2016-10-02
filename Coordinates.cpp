// $Id: Coordinates.cc 2641 2007-09-02 21:31:02Z flaterco $

/*  Coordinates   Degrees latitude and longitude.

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

#include "Coordinates.h"
#include <QRegularExpression>
#include <QVector>
#include <QDebug>

using namespace Tide;

Coordinates Coordinates::fromWGS84LatLong(double lat, double lng) {
    if (lat > 90 || lat < -90 || lng > 180 || lng < -180) return Coordinates();
    return Coordinates(lat, lng, "WGS_84");
}

Coordinates Coordinates::parseISO6709(const QString& loc) {
    // +27.5916+086.5640+8850CRSWGS_84/
    QRegularExpression coord("([+-]\\d+(?:\\.\\d+)?)(.*)");
    QVector<double> cs;
    QRegularExpressionMatch m;
    QString rest;
    for (m = coord.match(loc); m.hasMatch(); m = coord.match(m.captured(2))) {
        cs.append(m.captured(1).toDouble());
        rest = m.captured(2);
    }
    if (cs.length() < 2) {
        return Coordinates();
    }

    double lat = cs[0];
    double lng = cs[1];
    if (lat > 90 || lat < -90 || lng > 180 || lng < -180) {
        return Coordinates();
    }

    QString datum("WGS_84");
    QRegularExpression datumRe("CRS([^/]+)(/.*)");
    m = datumRe.match(rest);
    if (m.hasMatch()) {
        datum = m.captured(1);
        rest = m.captured(2);
    }

    if (rest != "/") {
        return Coordinates();
    }

    return Coordinates(lat, lng, datum);
}

static double sexas(double r) {
    r = ::fabs(r);
    return  60 * (r - int(r));
}

ushort Coordinates::degreesLat() const {
    return ::fabs(m_Latitude);
}

ushort Coordinates::degreesLng() const {
    return ::fabs(m_Longitude);
}

ushort Coordinates::minutesLat() const {
    return sexas(m_Latitude);
}

ushort Coordinates::minutesLng() const {
    return sexas(m_Longitude);
}

ushort Coordinates::secondsLat() const {
    return sexas(sexas(m_Latitude));
}

ushort Coordinates::secondsLng() const {
    return sexas(sexas(m_Longitude));
}

QString Coordinates::print() {
    if (m_Datum == "Invalid") return "N/A";

    // 50°40'46"N 024°48'26"E
    QString s("%1°%2'%3\"%4");
    QString r;
    QChar z('0');
    r += s.arg(degreesLat(), 2, 10, z).arg(minutesLat(), 2, 10, z)
            .arg(secondsLat(), 2, 10, z).arg(northern() ? "N" : "S");
    r += " ";
    r += s.arg(degreesLng(), 3, 10, z).arg(minutesLng(), 2, 10, z)
         .arg(secondsLng(), 2, 10, z).arg(eastern() ? "E" : "W");

    return r;
}

QString Coordinates::toISO6709() {
    if (m_Datum == "Invalid") return "N/A";
    // +27.5916+086.5640CRSWGS_84/
    QChar z('0');
    QString s("%1%2%3%4CRS%5/"); // sign lat, abs lat, sign lng, abs lng, datum
    return s.arg(m_Latitude < 0 ? '-' : '+').arg(::fabs(m_Latitude), 0, 'f', 2, z)
            .arg(m_Longitude < 0 ? '-' : '+').arg(::fabs(m_Longitude), 0, 'f', 3, z).arg(m_Datum);
}
