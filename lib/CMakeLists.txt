
set(_sources
  Grid.cpp
)

set(_headers
  ${HEADER_DIR}/overtile/Grid.h
)

add_library(overtile STATIC ${_sources} ${_headers})

install(TARGETS overtile ARCHIVE DESTINATION lib)

