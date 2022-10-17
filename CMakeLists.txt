cmake_minimum_required(VERSION 3.0.0)
project(concurrency VERSION 0.1.0)

include(CTest)
enable_testing()
include("/home/vcpkg/scripts/buildsystems/vcpkg.cmake")
include_directories("/home/vcpkg/installed/x64-linux/include")
include_directories("/home/mynet/lib")
add_executable(concurrency main.cpp)
find_package(PkgConfig)

 
    pkg_check_modules(liburing REQUIRED IMPORTED_TARGET GLOBAL liburing>=2.0)
    
    target_link_libraries(concurrency PRIVATE PkgConfig::liburing)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2b  ")
include(CPack)
