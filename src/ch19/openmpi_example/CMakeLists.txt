cmake_minimum_required(VERSION 3.16)
project(openmpi-example LANGUAGES CXX)

find_package(MPI REQUIRED)
message(STATUS "MPI: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")

add_executable("${PROJECT_NAME}" main.cpp)

target_link_libraries("${PROJECT_NAME}" PRIVATE MPI::MPI_CXX)
