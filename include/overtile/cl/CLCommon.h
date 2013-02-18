/*
 * CLCommon.h: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, Ohio State University
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: CLCommon.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

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
