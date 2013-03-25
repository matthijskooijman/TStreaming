#ifndef __TSTREAMING_H
#define __TSTREAMING_H

#ifndef lengthof
#define lengthof(x) (sizeof(x) / sizeof(*x))
#endif

#include "Formatters.h"
#include "DoublePrint.h"
#include "Types.h"


/**
 * Use the << operator to print stuff. This makes it easier to print
 * multiple stuff after each other. Using a template here makes sure
 * that anything that Print knows about, can be printed.
 */
template <typename T, typename enable_if<!is_base_of<Printable, T>::value, int>::type = 0>
inline Print& operator<< (Print &p, const T &v) {
  p.print(v);
  return p;
}

/**
 * Optimization: For Printable and subclasses, directly call printTo.
 * This helps GCC to inline the printTo call in some cases.
 *
 * This optimization needs complex template magic, since just defining
 * an overloaded version of operator<< for Printable& is not enough: The
 * templated version above will be instantiated for all subclasses, so
 * the overloaded version will never be used. In addition, it seems that
 * in some cases, such a Printable& version would not be as efficient as
 * possible, since (at some significant point in the optimization
 * process) GCC doesn't know the exact kind of Printable passed, so it
 * can't inline the printTo virtual call.
 */
template <typename T, typename enable_if<is_base_of<Printable, T>::value, int>::type = 0>
inline Print& operator<< (Print &p, const T &v) {
  v.printTo(p);
  return p;
}

/**
 * Dummy null printer.
 */
class Null final : public Print {
  // Final to allow optimizing away the vtable in some cases
  virtual size_t write(uint8_t c) { return 1; }
};

/**
 * Class for printing newlines. This is implemented as a class instead
 * of simply a constant string so it can call println so diffent Print
 * instances can use different line ending characters.
 */
class Endl final : public Printable {
// Final to allow optimizing away the vtable in some cases
public:
  virtual inline size_t printTo(Print &p) const {
    return p.println();
  }
};

/**
 * Static instantiation of Endl, to allow code like:
 *   Serial << endl;
 */
static Endl endl;

#endif // __TSTREAMING_H

/* vim: set sw=2 sts=2 expandtab filetype=cpp: */
