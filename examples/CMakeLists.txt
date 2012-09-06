
add_executable(rician-3d rician-3d.cpp)
target_link_libraries(rician-3d OTCore LLVMSupport)

add_executable(rician-3d-no-r rician-3d-no-r.cpp)
target_link_libraries(rician-3d-no-r OTCore LLVMSupport)
