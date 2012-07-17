
set(_sources
  CLContext.cpp
)

set(_headers
  ${HEADER_DIR}/overtile/cl/CLCommon.h
  ${HEADER_DIR}/overtile/cl/CLContext.h
)


add_library(overtile-cl STATIC ${_sources} ${_headers})

install(TARGETS overtile-cl ARCHIVE DESTINATION lib)
