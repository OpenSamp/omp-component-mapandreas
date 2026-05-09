cmake_minimum_required(VERSION 3.19)

# Vendored open.mp-sdk subtrees still pin VERSION 2.x/3.0, which CMake 3.31+
# refuses. Force a policy floor for every sub-project so configure works.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "")

project(mapandreas)

set(MA_VERSION "v1.2.0")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(MA_VERSION="${MA_VERSION}")

if(MSVC)
	add_compile_options(/MP)
endif()

# Architecture selector. Default to host bitness; override with -DMA_ARCH=x86 for
# a 32-bit build (kept around in case anyone still hosts a legacy SA-MP runtime).
if(NOT DEFINED MA_ARCH)
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(MA_ARCH "x86")
	else()
		set(MA_ARCH "x64")
	endif()
endif()

if(MA_ARCH STREQUAL "x86" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	add_compile_options(-m32)
	add_link_options(-m32)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# open.mp SDK (interface target; propagates NOMINMAX, /fp:strict, glm, etc.)
add_subdirectory(lib/open.mp-sdk)

add_subdirectory(src)
