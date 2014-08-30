add_executable(Mpi main.cc)
set_target_properties(Mpi PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/debug)
set_target_properties(Mpi PROPERTIES OUTPUT_NAME Mpi${BUILD_POSTFIX})

target_link_libraries(Mpi ${CMAKE_SOURCE_DIR}/lib/mpich/lib/libmpi.a)
target_link_libraries(Mpi ${CMAKE_SOURCE_DIR}/lib/mpich/lib/libmpicxx.a)
target_link_libraries(Mpi ${CMAKE_SOURCE_DIR}/lib/mpich/lib/libpmpi.a)
