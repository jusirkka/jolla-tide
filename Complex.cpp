#include "Complex.h"

using namespace Tide;

Complex::Complex(double r, double i): x(r), y(i) {}

Complex::Complex(std::complex<double> z): x(z.real()), y(z.imag()) {}

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

std::complex<double> Complex::complex() const {
  return std::complex<double>(x, y);
}
