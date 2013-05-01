#ifndef __TSTREAMING_FORMATTERS_H
#define __TSTREAMING_FORMATTERS_H

#include "Tuple.h"

/* Helper functor to get the values from a tuple and pass them to a
 * printer function. It recursively calls itself to move all values out
 * of the tuple and in to the argument list, one by one.
 *
 * This is a class instead of a function so we can use partial
 * specialization to recurse on the tuple type.
 *
 * Note that this class is quite specific for printing, but I couldn't
 * get a more generic "apply" functor to work.
 */
template<typename Formatter, typename Tuple, typename... Args>
class PrintTuple {
public:
  size_t static print(Print &p, Tuple t, Args... args)
  {
    /* Strip the head from the tuple, pass any existing arguments along
     * and add the head of the tuple to the arguments. */
    return PrintTuple<Formatter, typename Tuple::tail_tuple_type, Args..., typename Tuple::head_type>
           ::print(p, t.tail(), args..., t.head());
  }
};

/* Base case with empty tuple */
template<typename Formatter, typename... Args>
class PrintTuple<Formatter, tuple<>, Args...> {
public:
	size_t static print(Print &p, tuple<>, Args... args) {
		return Formatter::printValue(p, args...);
	}
};

/**
 * This is a set of values values (usually just one) that knows how
 * format itself through a template argument. Don't use this one
 * directly, instead use the V() function to create one.
 *
 * The value is stored in a tuple, in order to allow multiple values of
 * different types (e.g. an array along with its length for example).
 *
 * The PrintTuple class is used to actually unpack the tuple again and
 * feed the values to Formatter::printValue().
 */
template <class Formatter, typename... Ts>
class FormattedValue final : public Printable {
// Final to allow optimizing away the vtable in some cases
public:
  explicit FormattedValue(Ts... vs) : values(vs...) { }
  virtual size_t printTo(Print& p) const{
    PrintTuple<Formatter, tuple<Ts...>>::print(p, values);
  }
  tuple<Ts...> values;
};

/**
 * Optimization: Specialized << operator, which directly calls PrintTuple.
 * This allows GCC to inline up to or including the Formatter:printValue
 * call, even with -Os. Without this, this needs a higher -O and -flto
 * in some cases.
 */
template <class Formatter, typename... Ts>
static inline Print& operator << (Print &p, const FormattedValue<Formatter, Ts...> v) {
  PrintTuple<Formatter, tuple<Ts...>>::print(p, v.values);
  return p;
}

/**
 * Format a value using the given Formatter type. This returns a
 * Printable container object that is intended to printed using a Print
 * object.
 *
 * For example:
 *
 * Serial << F<Hex>(10) << "\r\n";
 * Serial.print(F<Align<10>("Test:"));
 */
template <class Formatter, typename... Ts>
inline FormattedValue<Formatter, Ts...> V(Ts... vs) {return FormattedValue<Formatter, Ts...>(vs...); }

/**
 * Format a integer as a hexadecimal number, using the specified number
 * of bits. Works for a signed integers as well, as long as they're
 * positive (for negative integers, you just get (part of) the raw two's
 * complement representation.
 */
template <size_t bits>
class HexBits {
public:
  template <typename T>
  static size_t printValue(Print& p, T value) {
    size_t unused = sizeof(T) * 8 - bits;
    /* Clear any bits we're not supposed to use. Note that we shift 2 <<
     * bits - 1 instead of 1 << bits, to prevent shifting by the total
     * width when all bits are used. This should be the same, except
     * when bits is zero, which isn't supported anyway. */
    value &= ((T)2 << bits - 1) - 1;

    size_t res = 0;

    /* Print nibble by nibble */
    for (int nibble = (bits - 1) / 4; nibble >= 0; --nibble) {
      res += p.print((value >> (nibble * 4)) & 0xf, HEX);
    }

    return res;
  }
};

/* Hexadecimal formatting, with autodetected bitwidth. */
class Hex {
public:
  template <typename T>
  static size_t printValue(Print& p, T value) {
    return HexBits<sizeof(T) * 8>::printValue(p, value);
  }
};

/*
 * Helper class, use Number below instead of this class.
 */
template <unsigned base>
class NumberBase { /* Base here means base class, not e.g, base 16 */
protected:
  /* The actual implementation of printing numbers lives in this
   * superclass, so the compiler doesn't generate a separate
   * implementation for every number of digits.
   * base, not once for each set of parameters. The latter could cause a
   * lot of duplicate code. */
  template <typename T>
  static size_t printNumber(Print& p, T value, uint8_t digits) {
    uint8_t buf[digits];

    /* Deduce each digit in turn */
    for (uint8_t i = digits; i > 0; --i) {
      buf[i - 1] = "01234567890ABCDEF"[value % base];
      value /= base;
    }
    return p.write(buf, digits);
  }
};

template <uint8_t digits, unsigned base = 10>
class Number : protected NumberBase<base> {
public:
  template <typename T>
  static size_t printValue(Print& p, T value) {
    static_assert(base >= 2, "Base cannot be less than 2");
    static_assert(base <= 16, "Base cannot be more than 16");
    NumberBase<base>::printNumber(p, value, digits);
  }
};

typedef Number<10> Dec;

/*
 * Align a value to a given width, using the given padding character.
 * Can be combined with other Formatters, e.g.,:
 *
 * Serial << F<Align<20>>( F<Hex>(0xabcd));
 */
template <int width, char padding = ' '>
class Align {
public:
  template <typename T>
  static size_t printValue(Print& p, T value) {
    size_t len = p.print(value);
    while (len < width)
      len += p.write(padding);
  }
};

/**
 * Printer that skips "skip" characters at the start and then prints
 * only "width" characters.
 * Not intended to be used directly, use Truncate or Skip instead.
 */
template <int skip, int width>
class Truncator : public Print {
  size_t count;
  Print &p;
public:
  Truncator(Print &p) : p(p), count(0) {}
  virtual size_t write(uint8_t c) {
    size_t res = 0;
    if ((skip == 0 || count >= skip)
     && (width == 0 || count < width + skip)) {
      res = p.write(c);
      count += res;
    } else {
      count++;
    }
    return res;
  }
};

/*
 * Truncate a value to a given width
 * Can be combined with other Formatters, e.g.,:
 *
 * Serial << F<Align<10>>( F<Truncate<8>>("Very long string"));
 */
template <int width>
class Truncate {
public:
  template <typename T>
  static size_t printValue(Print& p, T value) {
    return Truncator<0, width>(p).print(value);
  }
};

/*
 * Skip a number of characters from a printed value.
 * Can be combined with other Formatters, e.g.,:
 *
 * Serial << F<Truncate<4>>( F<Skip<2>>("__1234__"));
 */
template <int skip>
class Skip {
public:
  template <typename T>
  static size_t printValue(Print& p, T value) {
    return Truncator<skip, 0>(p).print(value);
  }
};

/*
 * Helper class, use Fixed below instead of this class.
 */
class FixedBase {
protected:
  /* The actual implementation of printing fixed values lives in this
   * superclass, so it gets generated only once for each value type, not
   * once for each set of parameters. The latter could cause a lot of
   * duplicate code. */
  template <typename T>
  static size_t printFixed(Print &p, T value, size_t scale, size_t precision) {
    size_t res = 0;
    /* Print the integer part */
    res += p.print(value / scale, DEC);
    res += p.print(".");
    value %= scale;
    for (size_t i = 0; i < precision; ++i) {
      value *= 10;
      res += p.print(value / scale, DEC);
      value %= scale;
    }

    return res;
  }
};

/**
 * Format a fixed point number as a decimal number with a given
 * precision.
 *
 * The value represented by the fixed point number is value * 1/scale.
 *
 * The precision is the number of decimal digits to output.
 */
template <size_t scale, size_t precision>
class Fixed : protected FixedBase {
public:
  template <typename T>
  static size_t printValue(Print& p, T value) {
    return printFixed(p, value, scale, precision);
  }
};

/**
 * Print a number as a character (i.e., print the ASCII character
 * associated with the value, so 0x65 becomes 'A').
 */
class Char {
public:
  template <typename T>
  static size_t printValue(Print &p, T value) {
    return p.write(value);
  }
};

/**
 * Don't apply special formatting, just print a value. This effectively
 * means that the Print class gets to decide how to format the value.
 *
 * Normally, this should print strings as expected and numerical types
 * in decimal, except for char, which is printed as the character it
 * represents.
 *
 * Usually you will not need this formatter, since the effect is the
 * same as just passing the value to Print directly. However, it might
 * be useful as an argument to other formatters that need a second
 * formatter as an argument.
 */
class NoFormat {
public:
  template <typename T>
  static size_t printValue(Print& p, T value) {
    p.print(value);
  }
};


/**
 * Print an array of given length.
 *
 * Each array element is formatted with the given formatter, and
 * elements are separated with up to two separator characters (set to 0
 * to hide the separator).
 */
template <class Formatter = NoFormat, char separator1 = ',', char separator2 = ' '>
class Array {
public:
  template <typename T>
  static size_t printValue(Print& p, const T *array, size_t len) {
    size_t res = 0;
    for (size_t i = 0; i < len; ++i) {
      if (i != 0 && separator1)
        p.print(separator1);
      if (i != 0 && separator2)
        p.print(separator1);
      res += p.print(V<Formatter>(array[i]));
    }
  }
};

#endif // __TSTREAMING_FORMATTERS_H

/* vim: set sw=2 sts=2 expandtab filetype=cpp: */
