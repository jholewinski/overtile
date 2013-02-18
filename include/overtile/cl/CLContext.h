/*
 * CLContext.h: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, University of California Los Angeles
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: CLContext.h
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#ifndef CLCONTEXT_HPP_INC
#define CLCONTEXT_HPP_INC 1

namespace overtile {

/**
 * Container for an OpenCL context and related data.
 */
class CLContext {
public:
  
  CLContext();

  ~CLContext();

  cl::Platform& platform() { return platform_; }
  cl::Context& context() { return context_; }
  cl::Device& device() { return device_; }
  
  static void throwOnError(const char* where,
                           cl_int result) throw (std::runtime_error);

private:

  cl::Platform platform_;
  cl::Context  context_;
  cl::Device   device_;
  
};

}

#endif
