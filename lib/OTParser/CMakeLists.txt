
add_bison_target(SSP SSPGrammar.y SSPParserGenerated.cpp SSPParserGenerated.h)
add_llvm_library(OTParser
  SSPParser.cpp
  SSPParserGenerated.cpp
)

install(TARGETS OTParser ARCHIVE DESTINATION lib)
