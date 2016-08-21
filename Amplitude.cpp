/*  $Id: Amplitude.cc 2641 2007-09-02 21:31:02Z flaterco $

    Amplitude:  A quantity in units of feet, meters,
    knots, or knots squared.  See also, PredictionValue.

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

#include "Amplitude.h"
#include <math.h>


using namespace Tide;

Amplitude Amplitude::fromDottedMeters(double m, int deriv) {
    return Amplitude(m, 1, - deriv);
}

Amplitude Amplitude::pow(double m, int ld, int td, int p) {
    return Amplitude(::pow(m, double (p)), ld*p, td*p);
}

Amplitude (Tide::operator*) (const Amplitude& a, const Amplitude& b) {
    return Amplitude(a.value * b.value, a.L + b.L, a.T + b.T);
}

static void throw_if_mismatch(const Amplitude& a, const Amplitude& b) {
    if (a.L != b.L || a.T != b.T ) {
        QString ad = QString("Dimensions (%1, %2)").arg(a.L, a.T);
        QString bd = QString("Dimensions (%1, %2)").arg(b.L, b.T);
        throw DimensionMismatch(ad + QString(" do not match ") + bd);
    }
}

Amplitude& Amplitude::operator+= (const Amplitude& a) {
    throw_if_mismatch(*this, a);
    value += a.value;
    return *this;
}

Amplitude& Amplitude::operator-= (const Amplitude& a) {
    throw_if_mismatch(*this, a);
    value -= a.value;
    return *this;
}

Amplitude& Amplitude::operator*= (double a) {
    value *= a;
    return *this;
}

Amplitude& Amplitude::operator- () {
    value = - value;
    return *this;
}


Amplitude (Tide::operator+) (const Amplitude& a, const Amplitude& b) {
    throw_if_mismatch(a, b);
    return Amplitude(a.value + b.value, a.L, a.T);
}

Amplitude (Tide::operator-) (const Amplitude& a, const Amplitude& b) {
    throw_if_mismatch(a, b);
    return Amplitude(a.value - b.value, a.L, a.T);
}

Amplitude (Tide::operator*) (const Amplitude& a, double b) {
    return Amplitude(a.value * b, a.L, a.T);
}

Amplitude (Tide::operator*) (double b, const Amplitude& a) {
    return a * b;
}

bool (Tide::operator>) (const Amplitude& a, const Amplitude& b) {
    throw_if_mismatch(a, b);
    return a.value > b.value;
}

bool (Tide::operator==) (const Amplitude& a, const Amplitude& b) {
    throw_if_mismatch(a, b);
    return a.value == b.value;
}
