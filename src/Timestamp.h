// $Id: Timestamp.hh 5029 2013-07-07 21:29:04Z flaterco $

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

#ifndef TIMESTAMP_H
#define TIMESTAMP_H


#include <exception>

#include "Interval.h"

#include <QString>


namespace Tide {

typedef qint64 timestamp_rep_t;


class Year;

class BadStamp: public std::exception
{
public:

    BadStamp(const QString& s): m_Message(s) {}

    const char* what() const throw() {
        return m_Message.toLatin1().constData();
    }

    ~BadStamp() throw() {}

private:

    QString m_Message;
};

class Timestamp {
public:

    static Timestamp fromPosixTime(timestamp_rep_t t);
    // Create a Timestamp for the beginning of the specified year in UTC
    // (YEAR-01-01 00:00:00Z)
    static Timestamp fromUTCYear(Year year);
    static Timestamp fromJulianDate(double julianDate);

    static Timestamp now();

    Timestamp(const Timestamp& a): m_PosixTime(a.m_PosixTime) {}
    Timestamp& operator=(const Timestamp& a) {m_PosixTime = a.m_PosixTime; return *this;}
    Timestamp(): m_PosixTime(0) {}


    bool isnil() {return m_PosixTime == 0;}

    // Convert to Julian date
    double jd() const;
    timestamp_rep_t posix() const;
    // Return the UTC year that this timestamp falls in
    Year year() const;

    Timestamp& operator+= (const Interval& a);

    QString print() const;

private:

    Timestamp(timestamp_rep_t t): m_PosixTime(t) {}

    timestamp_rep_t m_PosixTime;

};

bool operator<= (const Timestamp& a, const Timestamp& b);
bool operator< (const Timestamp& a, const Timestamp& b);
bool operator>= (const Timestamp& a, const Timestamp& b);
bool operator> (const Timestamp& a, const Timestamp& b);
bool operator!= (const Timestamp& a, const Timestamp& b);

Timestamp operator+ (const Timestamp& a, const Interval& b);
Timestamp operator- (const Timestamp& a, const Interval& b);

}
#endif
