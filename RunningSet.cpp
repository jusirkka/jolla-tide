#include "RunningSet.h"

using namespace Tide;


RunningSet::RunningSet(const Timestamp& epoch, const Amplitude& datum):
    ConstituentSet(),
    m_Epoch(epoch),
    m_Datum(datum)
{
}

Amplitude RunningSet::datum() const {return m_Datum;}

Amplitude RunningSet::tideDerivative(const Timestamp& t, unsigned deriv) const {
    Amplitude dt_tide = Amplitude::pow(0, m_Datum.L, m_Datum.T - deriv, 1); // set correct units
    Angle dt_phase = Angle::fromRadians(M_PI / 2.0 * deriv);

    Interval dt = t - m_Epoch;
    foreach (Data c, m_Constituents) {
        Angle arg = dt_phase + c.w * dt + c.p;
        dt_tide += Amplitude::pow(c.w.radiansPerSecond, 0, -1, deriv) * c.a * arg.cos();
    }
    return dt_tide;
}

Amplitude RunningSet::tideDerivativeMax(unsigned deriv) const {
    Amplitude dt_tide = Amplitude::pow(0, m_Datum.L, m_Datum.T - deriv, 1); // set correct units

    foreach (Data c, m_Constituents) {
        dt_tide += Amplitude::pow(c.w.radiansPerSecond, 0, -1, deriv) * c.a;
    }
    return dt_tide * 1.1;
}

RunningSet::~RunningSet() {}

void RunningSet::append(const Amplitude& a, const Speed& w, const Angle& p) {
    m_Constituents.append(Data(a, w, p));
}

void RunningSet::append(const Complex& c, const Speed& w) {
    m_Constituents.append(Data(m_Datum.unit() * c.mod(), w, Angle::fromRadians(c.arg())));
}
