// $Id: Timestamp.cc 5041 2013-07-10 01:54:08Z flaterco $

// Timestamp:  A point in time.  See also Year, Date, Interval.

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

#include <QDateTime>

#include "Timestamp.h"
#include "Year.h"


using namespace Tide;

Timestamp Timestamp::fromPosixTime(timestamp_rep_t t) {
    return Timestamp(t);
}


Timestamp Timestamp::now() {
    return Timestamp(QDateTime::currentMSecsSinceEpoch() / 1000);
}


// The beginning of time (1970-01-01 00:00:00Z) as a Julian date.
const static double c_epochJD(2440587.5);
const static double c_daySeconds(86400);

// Create a Timestamp for the specified Julian date.
Timestamp Timestamp::fromJulianDate(double julianDate) {
    timestamp_rep_t t = (julianDate - c_epochJD) * c_daySeconds;
    return Timestamp(t);
}

// Create a Timestamp for the beginning of the specified year in UTC
// (YEAR-01-01 00:00:00Z)
Timestamp Timestamp::fromUTCYear(Year year) {
    QDateTime dt(QDate(year.ad, 1, 1), QTime(0, 0), Qt::UTC);
    if (!dt.isValid()) {
        throw BadStamp(QString("Invalid DateTime from year %1").arg(year.ad));
    }
    return Timestamp(dt.toMSecsSinceEpoch() / 1000);
}


// Convert to Julian date.
double Timestamp::jd() const {
    return m_PosixTime / c_daySeconds + c_epochJD;
}


Year Timestamp::year() const {
    return Year::fromPosixTime(m_PosixTime);
}

timestamp_rep_t Timestamp::posix() const {
    return m_PosixTime;
}


Timestamp& Timestamp::operator+= (const Interval& a) {
    m_PosixTime += a.seconds;
    return *this;
}

bool (Tide::operator<=) (const Timestamp& a, const Timestamp& b) {
    return a.posix() <= b.posix();
}

bool (Tide::operator<) (const Timestamp& a, const Timestamp& b) {
    return a.posix() < b.posix();
}

bool (Tide::operator>=) (const Timestamp& a, const Timestamp& b) {
    return a.posix() >= b.posix();
}

bool (Tide::operator>) (const Timestamp& a, const Timestamp& b) {
    return a.posix() > b.posix();
}

Timestamp (Tide::operator+) (const Timestamp& a, const Interval& b) {
    return Timestamp::fromPosixTime(a.posix() + b.seconds);
}

bool (Tide::operator!=) (const Timestamp& a, const Timestamp& b) {
    return a.posix() != b.posix();
}
