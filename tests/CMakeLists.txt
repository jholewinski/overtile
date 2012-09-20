

## Setup test target

find_package(CUDA)
if (NOT CUDA_FOUND)
  message(WARNING "CUDA not found, test harness will not be configured")
endif()

find_package(PythonInterp)
if (NOT PYTHONINTERP_FOUND)
  message(WARNING "Python not found, test harness will not be configured")
endif()


if (CUDA_FOUND AND PYTHONINTERP_FOUND)

  set(OT_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  set(OT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")

  find_program(NVCC_BIN nvcc PATHS "${CUDA_SDK_ROOT_DIR}")
  if (NOT NVCC_BIN)
    message(FATAL_ERROR "Found CUDA SDK, but could not locate nvcc!")
  endif()  

  get_target_property(OTSC_BIN otsc LOCATION)

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/run-tests.py.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/run-tests.py"
                 @ONLY)

  add_custom_target(test
                    COMMAND ${PYTHON_EXECUTABLE} run-tests.py
                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                    COMMENT "Running regression tests"
                    DEPENDS otsc)

endif()

