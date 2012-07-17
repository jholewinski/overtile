
#ifndef CLCOMMON_HPP_INC
#define CLCOMMON_HPP_INC 1

// System Headers
#include <sys/time.h>

// C++ headers
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

// CL headers
#include "overtile/cl/cl.hpp"

inline double rtclock() {
  struct timeval  Tp;
  gettimeofday(&Tp, NULL);
  return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

template <typename T>
void printValue(const char* name, T value) {
  std::cout.width(20);
  std::cout << name << ": " << value << "\n";
}


#endif
