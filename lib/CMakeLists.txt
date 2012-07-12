
set(_sources
  Field.cpp
  Grid.cpp
  Types.cpp
)

set(_headers
  ${HEADER_DIR}/overtile/Field.h
  ${HEADER_DIR}/overtile/Grid.h
  ${HEADER_DIR}/overtile/Types.h
)

add_library(overtile STATIC ${_sources} ${_headers})

install(TARGETS overtile ARCHIVE DESTINATION lib)

