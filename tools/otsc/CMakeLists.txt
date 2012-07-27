
add_llvm_executable(otsc
  otsc.cpp
)

target_link_libraries(otsc
  OTParser
  OTCore
  LLVMSupport)

install(TARGETS otsc RUNTIME DESTINATION bin)
