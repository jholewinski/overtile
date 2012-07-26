
add_llvm_library(OTCore
  BackEnd.cpp
  Error.cpp
  Expressions.cpp
  Field.cpp
  Function.cpp
  Grid.cpp
  OpenCLBackEnd.cpp
  Region.cpp
  Types.cpp
)

install(TARGETS OTCore ARCHIVE DESTINATION lib)
