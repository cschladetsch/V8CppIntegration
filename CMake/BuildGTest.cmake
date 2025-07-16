# BuildGTest.cmake - Build GTest from source with same compiler settings as V8

include(ExternalProject)

# Set proper C++ flags for building with V8's libc++
set(GTEST_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(GTEST_CXX_FLAGS "${GTEST_CXX_FLAGS} -stdlib=libc++")
endif()

# Download and build GTest with same compiler/settings as V8
ExternalProject_Add(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/googletest
    CMAKE_ARGS
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_FLAGS=${GTEST_CXX_FLAGS}
        -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/googletest-install
        -DBUILD_GMOCK=OFF
        -DINSTALL_GTEST=ON
        -Dgtest_force_shared_crt=ON
    BUILD_BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-install/lib/libgtest.a
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-install/lib/libgtest_main.a
)

# Create directory for include files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-install/include)

# Create imported targets
add_library(GTest::gtest STATIC IMPORTED GLOBAL)
set_target_properties(GTest::gtest PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/googletest-install/lib/libgtest.a
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/googletest-install/include"
)
add_dependencies(GTest::gtest googletest)

add_library(GTest::gtest_main STATIC IMPORTED GLOBAL)
set_target_properties(GTest::gtest_main PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/googletest-install/lib/libgtest_main.a
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/googletest-install/include"
)
add_dependencies(GTest::gtest_main googletest)

# When using Clang with built V8, ensure same libc++ settings
if(NOT USE_SYSTEM_V8 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_property(TARGET GTest::gtest APPEND PROPERTY
        INTERFACE_COMPILE_OPTIONS -stdlib=libc++
    )
    set_property(TARGET GTest::gtest APPEND PROPERTY
        INTERFACE_LINK_OPTIONS -stdlib=libc++ -fuse-ld=lld
    )
    set_property(TARGET GTest::gtest_main APPEND PROPERTY
        INTERFACE_COMPILE_OPTIONS -stdlib=libc++
    )
    set_property(TARGET GTest::gtest_main APPEND PROPERTY
        INTERFACE_LINK_OPTIONS -stdlib=libc++ -fuse-ld=lld
    )
endif()

set(GTest_FOUND TRUE)
set(GTEST_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/googletest-install/include)
message(STATUS "GTest will be built from source with V8-compatible settings")