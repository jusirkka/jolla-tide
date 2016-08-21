// $Id: Constituent.cc 2641 2007-09-02 21:31:02Z flaterco $

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

#include "Constituent.h"


Constituent::Constituent(const Speed& s,
                         const Amplitude& a,
                         const Angle& p,
                         const QList<Year>& years,
                         const QList<Angle>& equiliriumArguments,
                         const QList<double>& nodeFactors):
    speed(s),
    amplitude(a),
    phase(p),
    m_Years(years)
{
    if (equiliriumArguments.length() != years.length()) {
        throw UnsupportedConstituentCorrection(
                    QString("Number of equilibrium arguments %1 does not match number of years %2")
                    .arg(equiliriumArguments.length(), years.length()));
    }
    if (nodeFactors.length() != years.length()) {
        throw UnsupportedConstituentCorrection(
                    QString("Number of node factors %1 does not match number of years %2")
                    .arg(nodeFactors.length(), years.length()));
    }

    for (int i = 0; i < years.length(); i++) {
        m_EquiliriumArguments[years[i]] = equiliriumArguments[i];
        m_NodeFactors[years[i]] = nodeFactors[i];
    }
}

const QList<Year>& Constituent::years() const {
    return m_Years;
}

const Angle&  Constituent::equiliriumArgument(const Year& year) const {
    if (!m_EquiliriumArguments.contains(year)) {
        throw UnsupportedConstituentCorrection(QString("No year %1 in equilibrium arguments").arg(year.ad));
    }
    return m_EquiliriumArguments.constFind(year).value();
}

double Constituent::nodeFactor(const Year& year) const {
    if (!m_NodeFactors.contains(year)) {
        throw UnsupportedConstituentCorrection(QString("No year %1 in node factors").arg(year.ad));
    }
    return m_NodeFactors[year];
}
