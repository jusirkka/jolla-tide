#ifndef CONSTITUENT_SET_H
#define CONSTITUENT_SET_H


#include "Amplitude.h"
#include "Timestamp.h"

namespace Tide {

class ConstituentSet {
public:


    ConstituentSet() {}
    virtual ~ConstituentSet() {}


    virtual Amplitude datum() const = 0;

    // Calculate (deriv)th time derivative of the normalized tide (for
    // time in s).
    virtual Amplitude tideDerivative(const Timestamp& t, unsigned deriv) const = 0;


    // Return the maximum that the absolute value of the (deriv)th
    // derivative of the tide can ever attain, plus "a little safety
    // margin."  tideDerivativeMax(0) == maxAmplitude() * 1.1
    virtual Amplitude tideDerivativeMax(unsigned deriv) const = 0;

    bool isCurrent() const {return datum().T < 0;}
    bool markSet(const Amplitude& a) const {return datum().T  == a.T && datum().L == a.L;}


};
}

#endif
