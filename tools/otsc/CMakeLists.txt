
set(LLVM_LINK_COMPONENTS
  support
)

add_llvm_executable(otsc
  otsc.cpp
)
target_link_libraries(otsc OTCore)

install(TARGETS otsc RUNTIME DESTINATION bin)
