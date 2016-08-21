#include "Complex.h"

Complex::Complex(double r, double i): x(r), y(i) {}

Complex Complex::conjugate() const {
    return Complex(x, -y);
}

double Complex::mod2() const {
    return x*x + y*y;
}

double Complex::mod() const {
    return ::sqrt(mod2());
}


double Complex::arg() const {
    return ::atan2(y, x);
}