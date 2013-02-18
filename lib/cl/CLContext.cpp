/*
 * CLContext.cpp: This file is part of the OverTile project.
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
 * @file: CLContext.cpp
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#include "overtile/cl/CLCommon.h"
#include "overtile/cl/CLContext.h"

namespace overtile {

typedef std::vector<cl::Platform> PlatformVector;
typedef std::vector<cl::Device>   DeviceVector;
typedef std::vector<std::string>  StringVector;

CLContext::CLContext() {
  cl_int         result;
  PlatformVector allPlatforms;

  result = cl::Platform::get(&allPlatforms);
  throwOnError("cl::Platform::get", result);

  if(allPlatforms.size() == 0) {
    throw std::runtime_error("No platforms available");
  }

  platform_ = allPlatforms[0];

  //printValue("Platform", platform_.getInfo<CL_PLATFORM_NAME>());

  // Create context
  cl_context_properties cps[3] = {
    CL_CONTEXT_PLATFORM,
    (cl_context_properties)(platform_)(),
    0
  };

  context_ = cl::Context(CL_DEVICE_TYPE_GPU, cps, NULL, NULL, &result);
  throwOnError("cl::Context", result);

  DeviceVector allDevices = context_.getInfo<CL_CONTEXT_DEVICES>();

  if(allDevices.size() == 0) {
    throw std::runtime_error("No devices available");
  }

  device_ = allDevices[0];

  //printValue("Device", device_.getInfo<CL_DEVICE_NAME>());
}

CLContext::~CLContext() {
}

void CLContext::throwOnError(const char* where,
                             cl_int result) throw (std::runtime_error) {
  if(result != CL_SUCCESS) {
    std::cout << "OpenCL Error Code: " << result << "\n";
    throw std::runtime_error(where);
  }
}

}
