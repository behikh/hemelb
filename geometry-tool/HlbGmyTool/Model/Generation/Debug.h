// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELBSETUPTOOL_DEBUG_H
#define HEMELBSETUPTOOL_DEBUG_H

#include <iostream>

class DummyStream {};

#ifdef DEBUG

template <typename T>
DummyStream& operator<<(DummyStream& ds, const T& val) {
  std::cout << val;
  return ds;
}
inline DummyStream& operator<<(DummyStream& ds,
                               std::ostream& (*func)(std::ostream& os)) {
  std::cout << std::endl;
  return ds;
}

#else

template <typename T>
DummyStream& operator<<(DummyStream& ds, const T& val) {
  return ds;
}
inline DummyStream& operator<<(DummyStream& ds,
                               std::ostream& (*func)(std::ostream& os)) {
  return ds;
}

#endif  // DEBUG

DummyStream& Log();

#endif  // HEMELBSETUPTOOL_DEBUG_H
