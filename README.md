TStreaming
==========
TStreaming is a Arduino / AVR library that implements streaming I/O
(*i.e.*, using the `<<` operator with an implementation based on C++
templates).

This library is still a work in progress, so expect the API to change in
the future.

Rationale
=========
The existing printing facilities in the Arduino libraries (Print and
Printable classes) provide a consistent interface between different
targets (Serial, Ethernet, etc.), but are also fairly verbose. A typical
debug print could look like:

	Serial.print("Value: ");
	Serial.print(x);
	Serial.print(" (0x");
	Serial.print(x, HEX);
	Serial.println(")");

Which is a lot of code for just a line of debug output. A printf style
output could be used, but this is not type-safe (i.e., a mismatch
between format string and parameters cannot be detected by the type
checker) and this does not work for arbitrary Printable subclasses.

Instead, the `<<` operator used in standard C++ (`iostream`) provides a
good way to chain multiple values to output in a concise and type-safe
manner. This is what TStreaming implements. With the TStreaming library,
the above example can be written as:

	Serial << "Value: " << x << " (0x" << V<Hex>(x) << ")";

With the exception that this prints leading zeroes before the Hex value
(the fact that the standard Print class has no way to do this was really
the original annoyance that triggered writing the first piece of code
that later became this library...).


Another consideration when writing this library was that you often have
values that always need to be interpreted in a specific way. For
example, you would store an integer that represents a fixed point value,
where the integer value stores the number of 0.1 increments in the
intended value.

To prevent having to spell out how to print these kind of numbers
everytime, you want to define a custom format and then only specify that
format when printing such a value. To make this type-safe again (and
also allow "stacking" formatting directives in some cases), format
specifiers are all types. This allows you to do, for example:

	typedef Fixed<10, 2> SpecialNumber

	...

	Serial << V<SpecialNumber>(x);

But also:

	typedef Align<SpecialNumber, ...> AlignedSpecialNumber


TODO: More documentation and examples.
