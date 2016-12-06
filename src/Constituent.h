// $Id: Constituent.hh 2641 2007-09-02 21:31:02Z flaterco $

// Constituent:  All that which pertains to a specific constituent
// from a station viewpoint:  speed, equilibrium arguments, node
// factors, amplitude, and phase.

/*
    Copyright (C) 2006  David Flater.

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

#ifndef CONSTITUENT_H
#define CONSTITUENT_H

#include <QList>
#include <QMap>


#include "Speed.h"
#include "Year.h"
#include "Angle.h"
#include "Amplitude.h"

#include <exception>
#include <QString>

namespace Tide {
using namespace std;

class UnsupportedConstituentCorrection: public exception {
public:

    UnsupportedConstituentCorrection(const QString& msg): m_Message(msg) {}

    const char* what() const throw() {
        return m_Message.toLatin1().constData();
    }

    ~UnsupportedConstituentCorrection() throw() {}

private:

    QString m_Message;
};

class Constituent {
public:

    Speed speed;
    Amplitude amplitude;
    Angle phase;                          // A.k.a. epoch, -k'
    double nodeFactor(const Year& year) const;
    const Angle&  equiliriumArgument(const Year& year) const;
    const QList<Year>& years() const;

    Constituent(const Speed& speed,
                const Amplitude& amplitude,
                const Angle& phase,
                const QList<Year>& years,
                const QList<Angle>& equiliriumArguments,
                const QList<double>& nodeFactors);


private:

    QMap<Year, Angle> m_EquiliriumArguments;
    QMap<Year, double> m_NodeFactors;
    QList<Year> m_Years;

};
}
#endif
