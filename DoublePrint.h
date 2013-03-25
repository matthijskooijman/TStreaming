#ifndef __TSTREAMING_DOUBLE_RINT_H
#define __TSTREAMING_DOUBLE_PRINT_H

#include "Types.h"

/*
 * A printer that prints to two slave printers simultaneously.
 */
class DoublePrint : public Print {
private:
  DoublePrint(Print& a, Print& b) : a(a), b(b) {}

  /* Our slaves */
  Print &a, &b;

public:
  /* The core of this class. All other print methods are implemented by
   * our parent class. */
  virtual size_t write(uint8_t c) {
    return (a.write(c) + b.write(c)) / 2;
  }

  /* Optimization: Re-implement common Print methods, so the compiler
   * can optimize away the DoublePrint constructor and the below print
   * methods and compile
   *   (Foo & Bar) << "baz";
   * as if it were:
   *   Foo.print("baz");
   *   Bar.print("baz");
   *
   * Note that this produces slightly different behaviour than without
   * these methods.  Now, the print methods are called twice (including
   * any output conversion, e.g, formatting a hex number). Without the
   * below methods, the print methods are called just once and the
   * write(char) method is called twice for every character.
   *
   * This means that adding the below methods makes the code smaller,
   * but (in a lot of cases) slower.
   *
   * Templates are used to easily re-implement all methods.
   * */
  template <typename T, typename enable_if<!is_base_of<Printable, T>::value, int>::type = 0>
  size_t print(const T &v) {
    return (a.print(v) + b.print(v)) / 2;
  }

  template <typename T1, typename T2>
  size_t print(T1 v1, T2 v2) {
    return (a.print(v1, v2) + b.print(v1, v2)) / 2;
  }

  /**
   * Optimization: For Printable and subclasses, directly call printTo.
   * This helps GCC to inline the printTo call in some cases.
   */
  template <typename T, typename enable_if<is_base_of<Printable, T>::value, int>::type = 0>
  size_t print(const T &v) {
    return (v.printTo(b) + v.printTo(a)) / 2;
  }

  /* Specialize the << operator for us, without using references. The
   * generic << operator operates on Print references, but in an
   * expression like (a & b) << "foo"; the result of (a & b) is an
   * rvalue and cannot be put inside a reference.
   * Instead, we just use non-reference values and copy them (but the
   * compiler will know how to optimize the copies away).
   */
  template <typename T>
  inline DoublePrint operator<< (const T &v) {
          print(v);
          return *this;
  }

  /* Allow (only) the & operator to construct us */
  friend DoublePrint operator &(Print& a, Print& b);
};

/**
 * Combine two print targets into a single target.
 */
inline DoublePrint operator &(Print& a, Print& b) {
  return DoublePrint(a, b);
}

#endif // __TSTREAMING_DOUBLE_RINT_H

/* vim: set sw=2 sts=2 expandtab filetype=cpp: */
