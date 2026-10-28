module;
#include <cmath>
#include <concepts>
#include <numbers>
#include <string>
#include <vector>

export module cx.core;

export namespace cx {

template<typename T>
concept Field = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { a - b } -> std::convertible_to<T>;
    { a * b } -> std::convertible_to<T>;
    { a / b } -> std::convertible_to<T>;
    { -a }    -> std::convertible_to<T>;
    { T{} }   -> std::same_as<T>;
    { T{1} }  -> std::same_as<T>;
};

template<Field T = double>
struct Complex {
    T re{}, im{};

    constexpr Complex() = default;
    constexpr Complex(T r) : re(r), im{} {}
    constexpr Complex(T r, T i) : re(r), im(i) {}

    constexpr Complex operator+(const Complex& o) const { return {re+o.re, im+o.im}; }
    constexpr Complex operator-(const Complex& o) const { return {re-o.re, im-o.im}; }
    constexpr Complex operator*(const Complex& o) const {
        return {re*o.re - im*o.im, re*o.im + im*o.re};
    }
    constexpr Complex operator/(const Complex& o) const {
        T d = o.re*o.re + o.im*o.im;
        return {(re*o.re + im*o.im)/d, (im*o.re - re*o.im)/d};
    }
    constexpr Complex operator-() const { return {-re, -im}; }

    constexpr T abs2() const { return re*re + im*im; }
    T abs() const { return std::sqrt(abs2()); }
    T arg() const { return std::atan2(im, re); }

    Complex exp() const {
        T e = std::exp(re);
        return {e*std::cos(im), e*std::sin(im)};
    }
    Complex log() const { return {std::log(abs()), arg()}; }
    Complex pow(const Complex& p) const { return (p * log()).exp(); }

    Complex sin() const {
        return {std::sin(re)*std::cosh(im), std::cos(re)*std::sinh(im)};
    }
    Complex cos() const {
        return {std::cos(re)*std::cosh(im), -std::sin(re)*std::sinh(im)};
    }
};

} // namespace cx
