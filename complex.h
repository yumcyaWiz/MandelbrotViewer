#ifndef COMPLEX_H
#define COMPLEX_H

#include <cmath>


template <typename T>
struct Complex {
  T real;
  T imag;

  Complex() { real = imag = 0; };
  Complex(T _real, T _imag) : real(_real), imag(_imag) {};
};


template <typename T>
Complex<T> operator+(const Complex<T>& c1, const Complex<T>& c2) {
  return Complex<T>(c1.real + c2.real, c1.imag + c2.imag);
}
template <typename T>
Complex<T> operator-(const Complex<T>& c1, const Complex<T>& c2) {
  return Complex<T>(c1.real - c2.real, c1.imag - c2.imag);
}
template <typename T>
Complex<T> operator*(const Complex<T>& c1, const Complex<T>& c2) {
  return Complex<T>(c1.real*c2.real - c1.imag*c2.imag, c1.real*c2.imag + c2.real*c1.imag);
}
template <typename T>
Complex<T> operator/(const Complex<T>& c1, const Complex<T>& c2) {
  const T div = c2.real*c2.real + c2.imag*c2.imag;
  return Complex<T>((c2.real*c1.real + c2.imag*c1.imag)/div, (c2.real*c1.imag - c2.imag*c1.real)/div);
}


template <typename T>
double length(const Complex<T>& c) {
  return std::sqrt(c.real*c.real + c.imag*c.imag);
}


using Complex_d = Complex<double>;
using Complex_f = Complex<float>;
#endif
