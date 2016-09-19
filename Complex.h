#ifndef COMPLEX_H
#define COMPLEX_H

#include <cmath>

namespace Tide {
class Complex {
public:

    double x;
    double y;

    Complex(double r = 0.0, double i = 0.0);
    Complex(const Complex& a): x(a.x), y(a.y) {}
    Complex& operator=(const Complex& a) {x = a.x; y = a.y; return *this;}
    Complex& operator+=(const Complex& a) {x += a.x; y += a.y; return *this;}

    Complex conjugate() const;
    double mod2() const;
    double mod() const;
    double arg() const;

private:

};


inline Complex operator- (double a, const Complex& c) {
    return Complex(a  - c.x, c.y);
}

inline Complex operator+ (const Complex& c1, const Complex& c2) {
    return Complex(c1.x + c2.x, c1.y + c2.y);
}

inline Complex operator- (const Complex& c1, const Complex& c2) {
    return Complex(c1.x - c2.x, c1.y - c2.y);
}

inline Complex operator* (double a, const Complex& c) {
    return Complex(a * c.x, a * c.y);
}

inline Complex operator* (const Complex& z1, const Complex& z2) {
    return Complex(z1.x*z2.x - z1.y*z2.y, z1.x*z2.y + z1.y*z2.x);
}

inline Complex operator/ (const Complex& c, double a) {
    return Complex(c.x / a, c.y / a);
}

inline Complex operator/ (const Complex& z1, const Complex& z2) {
    return z1 * z2.conjugate() / z2.mod2();
}

inline const Complex exp(Complex c) {
  return ::exp(c.x) * Complex(::cos(c.y), ::sin(c.y));
}

}


#endif
