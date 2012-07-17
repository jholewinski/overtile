
set(_sources
  BackEnd.cpp
  CGExpressions.cpp
  Expressions.cpp
  Field.cpp
  Function.cpp
  Grid.cpp
  OpenCLBackEnd.cpp
  Region.cpp
  Types.cpp
)

set(_headers
  ${HEADER_DIR}/overtile/BackEnd.h
  ${HEADER_DIR}/overtile/CGExpressions.h
  ${HEADER_DIR}/overtile/Expressions.h
  ${HEADER_DIR}/overtile/Field.h
  ${HEADER_DIR}/overtile/Function.h
  ${HEADER_DIR}/overtile/Grid.h
  ${HEADER_DIR}/overtile/OpenCLBackEnd.h
  ${HEADER_DIR}/overtile/Region.h
  ${HEADER_DIR}/overtile/Types.h
)

add_library(overtile STATIC ${_sources} ${_headers})

install(TARGETS overtile ARCHIVE DESTINATION lib)


add_subdirectory(cl)

