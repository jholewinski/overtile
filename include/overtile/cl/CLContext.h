
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
