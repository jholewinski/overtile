
add_llvm_library(OTParser
  OTDParser.cpp
)

install(TARGETS OTParser ARCHIVE DESTINATION lib)
