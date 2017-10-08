#ifndef ABAKUS_NUMERICTYPES_H
#define ABAKUS_NUMERICTYPES_H
/*
 * numerictypes.h - part of abakus
 * Copyright (C) 2004, 2005 Michael Pyne <michael.pyne@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sstream>
#include <string>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>

#include "hmath.h"

#ifdef HAVE_MPFR
#include <mpfr.h>
#endif

namespace Abakus
{

/* What trigonometric mode we're in. */
typedef enum { Degrees, Radians } TrigMode;

/* Shared application-wide */
extern TrigMode m_trigMode;

/* Precision to display at. */
extern int m_prec;

/**
 * Representation of a number type.  Includes the basic operators, along with
 * built-in functions such as abs() and mod().
 *
 * You need to actually define it using template specializations though.  You
 * can add functions in a specialization, it may be worth it to have the
 * functions declared here as well so that you get a compiler error if you
 * forget to implement it.
 *
 * Note that since we're using a specialization, and then typedef'ing the
 * new specialized class to number_t, that means we only support one type of
 * number at a time, and the choice is made at compile-time.
 */
template <typename T>
class number
{
public:
    /// Default ctor and set-and-assign ctor wrapped in one.
    number(const T& t = T());

    /// Copy constructor.
    number(const number &other);

    /// Create number from textual representation, useful for ginormously
    /// precise numbers.
    number(const char *str);

    /// Likewise
    explicit number(const QByteArray &str);

    /// Convienience constructor to create a number from an integer.
    explicit number(int i);

    /// Assignment operator.  Be sure to check for &other == this if necessary!
    number<T> &operator =(const number<T> &other);

    // You need to implement the suite of comparison operators as well, along
    // with the negation operator.  Sorry.

    bool operator!=(const number<T> &other) const;
    bool operator==(const number<T> &other) const;
    bool operator<(const number<T> &other) const;
    bool operator>(const number<T> &other) const;
    bool operator<=(const number<T> &other) const;
    bool operator>=(const number<T> &other) const;

    number<T> operator -() const;

    // These functions must be implemented by all specializations to be used.
    // Note that when implementing these functions, the implicit value is the
    // value that this object is wrapping.  E.g. you'd call the function on
    // a number object, kind of like 3.sin() if you were using Ruby.

    // Trigonometric, must accept values in degrees.
    number<T> sin() const;
    number<T> cos() const;
    number<T> tan() const;

    // Inverse trigonometric, must return result in Degrees if necessary.
    number<T> asin() const;
    number<T> acos() const;
    number<T> atan() const;

    // Hyperbolic trigonometric (doesn't use Degrees).
    number<T> sinh() const;
    number<T> cosh() const;
    number<T> tanh() const;

    // Inverse hyperbolic trigonometric (doesn't use degrees).
    number<T> asinh() const;
    number<T> acosh() const;
    number<T> atanh() const;

    /// @return Number rounded to closest integer less than or equal to value.
    number<T> floor() const;

    /// @return Number rounded to closest integer greater than or equal to value.
    number<T> ceil() const;

    /// @return Number with only integer component of result.
    number<T> integer() const;

    /// @return Number with only fractional component of result.
    number<T> frac() const;

    /**
     * @return Number rounded to nearest integer.  What to do in 'strange'
     * situations is specialization-dependant, I don't really care enough to
     * mandate one or the other.
     */
    number<T> round() const;

    /// @return Absolute value of number.
    number<T> abs() const;

    /// @return Square root of number.
    number<T> sqrt() const;

    /// @return Natural-base logarithm of value.
    number<T> ln() const;

    /// @return base-10 logarithm of value.
    number<T> log() const;

    /// @return Natural base raised to the power given by our value.
    number<T> exp() const;

    /// @return Our value raised to the \p exponent power.  Would be nice if
    /// it supported even exponents on negative numbers correctly.
    number<T> pow(const number<T> &exponent);

    /// @return value rounded to double precision.
    double asDouble() const;

    /// @return Textual representation of the number, adjusted to the user's
    /// current precision.
    QString toString() const;

    /// @return Our value.
    T value() const;
};

// You should also remember to overload the math operators for your
// specialization.  These generic ones should work for templates wrapping a
// type that C++ already has operators for.

template<typename T>
inline number<T> operator+(const number<T> &l, const number<T> &r)
{
    return number<T>(l.value() + r.value());
}

template<typename T>
inline number<T> operator-(const number<T> &l, const number<T> &r)
{
    return number<T>(l.value() - r.value());
}

template<typename T>
inline number<T> operator*(const number<T> &l, const number<T> &r)
{
    return number<T>(l.value() * r.value());
}

template<typename T>
inline number<T> operator/(const number<T> &l, const number<T> &r)
{
    return number<T>(l.value() / r.value());
}

// Defined in numerictypes.cpp for ease of reimplementation.
QString convertToString(const HNumber &num);

/**
 * Specialization for internal HMath library, used if MPFR isn't usable.
 *
 * @author Michael Pyne <michael.pyne@kdemail.net>
 */
template<>
class number<HNumber>
{
public:
    typedef HNumber value_type;

    number(const HNumber& t = HNumber()) : m_t(t)
    {
    }
    explicit number(int i) : m_t(i) { }
    number(const number<HNumber> &other) : m_t(other.m_t) { }

    /// Likewise
    explicit number(const QByteArray &str) : m_t(str.constData()) { }

    number(const char *s) : m_t(s) { }

    bool operator!=(const number<HNumber> &other) const
    {
        return m_t != other.m_t;
    }

    bool operator==(const number<HNumber> &other) const
    {
        return m_t == other.m_t;
    }

    bool operator<(const number<HNumber> &other) const
    {
        return m_t < other.m_t;
    }

    bool operator>(const number<HNumber> &other) const
    {
        return m_t > other.m_t;
    }

    bool operator<=(const number<HNumber> &other) const
    {
        return m_t <= other.m_t;
    }

    bool operator>=(const number<HNumber> &other) const
    {
        return m_t >= other.m_t;
    }

    number<HNumber> &operator=(const number<HNumber> &other)
    {
        m_t = other.m_t;
        return *this;
    }

    HNumber asRadians() const
    {
        if(m_trigMode == Degrees)
            return m_t * PI / HNumber("180.0");
        else
            return m_t;
    }

    HNumber toTrig(const HNumber &num) const
    {
        // Assumes num is in radians.
        if(m_trigMode == Degrees)
            return num * HNumber("180.0") / PI;
        else
            return num;
    }

    number<HNumber> sin() const
    {
        return HMath::sin(asRadians());
    }

    number<HNumber> cos() const
    {
        return HMath::cos(asRadians());
    }

    number<HNumber> tan() const
    {
        return HMath::tan(asRadians());
    }

    number<HNumber> asin() const
    {
        return toTrig(HMath::asin(m_t));
    }

    number<HNumber> acos() const
    {
        return toTrig(HMath::acos(m_t));
    }

    number<HNumber> atan() const
    {
        return toTrig(HMath::atan(m_t));
    }

    number<HNumber> floor() const
    {
        if(HMath::frac(m_t) == HNumber("0.0"))
            return integer();
        if(HMath::integer(m_t) < HNumber("0.0"))
            return HMath::integer(m_t) - 1;
        return integer();
    }

    number<HNumber> ceil() const
    {
        return floor().value() + HNumber(1);
    }

/* There is a lot of boilerplate ahead, so define a macro to declare and
 * define some functions for us to forward the call to HMath.
 */
#define DECLARE_IMPL(name) number<value_type> name() const \
{ return HMath::name(m_t); }

    DECLARE_IMPL(frac)
    DECLARE_IMPL(integer)
    DECLARE_IMPL(round)

    DECLARE_IMPL(abs)

    DECLARE_IMPL(sqrt)

    DECLARE_IMPL(ln)
    DECLARE_IMPL(log)
    DECLARE_IMPL(exp)

    DECLARE_IMPL(sinh)
    DECLARE_IMPL(cosh)
    DECLARE_IMPL(tanh)

    DECLARE_IMPL(asinh)
    DECLARE_IMPL(acosh)
    DECLARE_IMPL(atanh)

    HNumber value() const { return m_t; }

    double asDouble() const { return toString().toDouble(); }

    number<HNumber> operator-() const { return HMath::negate(m_t); }

    // TODO: I believe this doesn't work for negative numbers with even
    // exponents.  Which breaks simple stuff like (-2)^2. :(
    number<HNumber> pow(const number<HNumber> &exponent)
    {
        return HMath::raise(m_t, exponent.m_t);
    }

    QString toString() const
    {
        return convertToString(m_t);
    }

    static number<HNumber> nan()
    {
        return HNumber::nan();
    }

    static const HNumber PI;
    static const HNumber E;

private:
    HNumber m_t;
};

    // Abakus namespace continues.
    typedef number<HNumber> number_t;

}; // namespace Abakus

#endif /* ABAKUS_NUMERICTYPES_H */

// vim: set et ts=8 sw=4:
