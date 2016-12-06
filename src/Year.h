// $Id: Year.hh 2641 2007-09-02 21:31:02Z flaterco $

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

#ifndef YEAR_H
#define YEAR_H

#include <exception>
#include <QString>
#include "Timestamp.h"

namespace Tide {

typedef qint16 year_rep_t;

class UnsupportedAD: public std::exception
{
public:

    UnsupportedAD(year_rep_t y): m_AD(y) {}

    const char* what() const throw() {
        return QString("Invalid year %1").arg(m_AD).toLatin1().constData();
    }

private:

    year_rep_t m_AD;
};


class Year {
public:

    friend Year operator+ (const Year& a, int b);
    friend Year operator- (const Year& a, int b);

    year_rep_t ad;

    static Year fromAD(year_rep_t);
    static Year fromPosixTime(timestamp_rep_t t);
    static Year currentYear();

    Year(const Year& a): ad(a.ad) {}
    Year& operator=(const Year& a) {ad = a.ad; return *this;}
    Year(): ad(1970) {}


private:

    Year(year_rep_t y): ad(y) {}

};


Year operator+ (const Year& a, int b);
Year operator- (const Year& a, int b);


bool operator< (const Year& y1, const Year& y2);
bool operator!= (const Year& y1, const Year& y2);

}
#endif

