#ifndef __TSTREAMING_H
#define __TSTREAMING_H

#ifndef lengthof
#define lengthof(x) (sizeof(x) / sizeof(*x))
#endif

#include "Formatters.h"
#include "DoublePrint.h"


/**
 * Use the << operator to print stuff. This makes it easier to print
 * multiple stuff after each other. Using a template here makes sure
 * that anything that Print knows about, can be printed.
 */
template <typename T>
inline Print& operator<< (Print &p, const T &v) {
  p.print(v);
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
 * Optimization: Specialized << operator, which can be inlined even with
 * -Os (the indirect route through Print::print and printTo needs more
 *  extensive (type) analysis and inlining which isn't applied with -Os.
 */
static inline Print& operator << (Print &p, const Endl &e) {
  e.printTo(p);
  return p;
}

static Endl endl;

#endif // __TSTREAMING_H

/* vim: set sw=2 sts=2 expandtab filetype=cpp: */
