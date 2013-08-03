xtd
===

Some frequently used extensions to the C++ standard library.

This project arose from a need to use some basic extensions to the STD in multiple projects. Some are minor extensions (e.g. utility overloads or ADL arrangements), others are new features (e.g. unformatted iostream manipulators) or "bug fixes" for the current standard (e.g. make_unique which wasn't included in C++11). It is targeted at C++11 compliant compilers and STD implementations. It may also include content which is possibly scheduled for the next C++ standard library with xtd acting as a bridging solution. These are likely to be removed once they next standard provides them natively.

The headers are organized according to the STD, so if a header adds extensions to iostream manipulators it is implemented in the header<xtd/iomanip>. If it adds ADL overloads for std::pair it is inside <xtd/utility>, and so forth.

It is currently a header-only library, though this may change in the future if necessary.
