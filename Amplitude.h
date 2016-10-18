/*  $Id: Amplitude.hh 2641 2007-09-02 21:31:02Z flaterco $

    Amplitude:  A non-negative quantity in units of feet, meters,
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


#ifndef AMPLITUDE_H
#define AMPLITUDE_H

#include <exception>
#include <QString>

namespace Tide {

class DimensionMismatch: public std::exception
{
public:

    DimensionMismatch(const QString& msg): m_Message(msg) {}

    const char* what() const throw() {
        return m_Message.toLatin1().constData();
    }


    ~DimensionMismatch() throw() {}

private:

    QString m_Message;
};



class Amplitude {
public:


    friend Amplitude operator* (const Amplitude& a, const Amplitude& b);
    friend Amplitude operator+ (const Amplitude& a, const Amplitude& b);
    friend Amplitude operator- (const Amplitude& a, const Amplitude& b);
    friend Amplitude operator* (const Amplitude& a, double b);
    friend Amplitude operator* (double b, const Amplitude& a);

    Amplitude& operator+= (const Amplitude& a);
    Amplitude& operator-= (const Amplitude& a);
    Amplitude& operator*= (double a);
    Amplitude& operator- (); // unary minus

    double value;
    int L, T;

    static Amplitude fromDottedMeters(double, int);
    static Amplitude parseDottedMeters(const QString& s);
    static Amplitude pow(double m, int l, int t, int p);

    Amplitude(const Amplitude& a): value(a.value), L(a.L), T(a.T) {}
    Amplitude& operator=(const Amplitude& a) {value = a.value; L = a.L; T = a.T; return *this;}
    Amplitude(): value(0), L(0), T(0) {}


    Amplitude null() {return Amplitude(0, L, T);}
    Amplitude unit() {return Amplitude(1, L, T);}

    QString print() const;
    bool valid() const {return L != 0 || T != 0;}

private:

    Amplitude(double m, int l, int t): value(m), L(l), T(t) {}

};

Amplitude operator* (const Amplitude& a, const Amplitude& b);
Amplitude operator+ (const Amplitude& a, const Amplitude& b);
Amplitude operator- (const Amplitude& a, const Amplitude& b);
Amplitude operator* (const Amplitude& a, double b);
Amplitude operator* (double b, const Amplitude& a);

bool operator> (const Amplitude& a, const Amplitude& b);
bool operator== (const Amplitude& a, const Amplitude& b);

}

#endif
