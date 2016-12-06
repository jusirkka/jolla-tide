#ifndef RUNNINGSET_H
#define RUNNINGSET_H

#include "ConstituentSet.h"
#include "Timestamp.h"
#include "Amplitude.h"
#include "Speed.h"
#include "Angle.h"
#include "Complex.h"

#include <QList>

namespace Tide {

class RunningSet : public ConstituentSet
{
public:

    RunningSet(const Timestamp& epoch, const Amplitude& datum);

    Amplitude datum() const;
    Amplitude tideDerivative(const Timestamp& t, unsigned deriv) const;
    Amplitude tideDerivativeMax(unsigned deriv) const;

    virtual ~RunningSet();

    void append(const Amplitude& a, const Speed& w, const Angle& phase);
    void append(const Complex& c, const Speed& w);

private:

    class Data {
    public:
        Data(const Amplitude& A, const Speed& W, const Angle& P): a(A), w(W), p(P) {}
        Amplitude a;
        Speed w;
        Angle p;
    };

    Timestamp m_Epoch;
    Amplitude m_Datum;
    QList<Data> m_Constituents;

};

}
#endif // RUNNINGSET_H
