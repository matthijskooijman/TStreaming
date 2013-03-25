#ifndef __TSTREAMING_DOUBLE_RINT_H
#define __TSTREAMING_DOUBLE_PRINT_H

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

  /* Specialize the << operator for us, without using references. The
   * generic << operator operates on Print references, but in an
   * expression like (a & b) << "foo"; the result of (a & b) is an
   * rvalue and cannot be put inside a reference.
   * Instead, we just use non-reference values and copy them (but the
   * compiler will know how to optimize the copies away).
   */
  template <typename T>
  inline DoublePrint operator<< (T v) {
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
