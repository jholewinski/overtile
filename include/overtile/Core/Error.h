/*
 * Error.h: This file is part of the OverTile project.
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
 * @file: Error.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#ifndef OVERTILE_CORE_ERROR_H
#define OVERTILE_CORE_ERROR_H

#include "llvm/Support/system_error.h"

namespace overtile {

// Error handling plumbing
const llvm::error_category &overtile_category();

struct overtile_error {
  enum _ {
    success = 0,
    ssp_parse_error,
  };
  _ V_;

  overtile_error(_ v) : V_(v) {}
  explicit overtile_error(int v) : V_(_(v)) {}
  operator int() const { return V_; }
};

inline llvm::error_code make_error_code(overtile_error e) {
  return llvm::error_code(static_cast<int>(e), overtile_category());
}
}

namespace llvm {
template <> struct is_error_code_enum<overtile::overtile_error> : true_type {};
template <>
struct is_error_code_enum<overtile::overtile_error::_> : true_type {};
}

#endif
