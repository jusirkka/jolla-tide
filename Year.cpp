// $Id: Year.cc 2641 2007-09-02 21:31:02Z flaterco $

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

#include "Year.h"
#include <QDateTime>

using namespace Tide;

Year Year::fromAD(year_rep_t y) {
    return Year(y);
}

Year Year::fromPosixTime(timestamp_rep_t t) {
    year_rep_t y = QDateTime::fromMSecsSinceEpoch(t * 1000).toUTC().date().year();
    return fromAD(y);
}

Year Year::currentYear() {
    year_rep_t y = QDateTime::currentDateTime().toUTC().date().year();
    return fromAD(y);
}


Year operator+ (const Year& a, int b) {
    return Year::fromAD(a.ad + b);
}

Year operator- (const Year& a, int b) {
    return Year::fromAD(a.ad - b);
}

bool operator< (const Year& y1, const Year& y2) {return y1.ad < y2.ad;}
bool operator!= (const Year& y1, const Year& y2) {return y1.ad != y2.ad;}
