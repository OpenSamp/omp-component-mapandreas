set(PLUGIN_SOURCES
	${PROJECT_SOURCE_DIR}/lib/pawn/amxplugin.cpp
	MapAndreas.cpp
	natives.cpp
	openmp_component.cpp
)

add_library(${PROJECT_NAME} MODULE ${PLUGIN_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	PREFIX ""
	# Force empty DEBUG_POSTFIX so the artifact stays mapandreas.{dll,so}
	# regardless of build configuration — matches the runtime expectation.
	DEBUG_POSTFIX ""
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_source_files_properties(${PROJECT_SOURCE_DIR}/lib/pawn/amxplugin.cpp PROPERTIES
	SKIP_PRECOMPILE_HEADERS ON
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/lib/pawn
	${PROJECT_SOURCE_DIR}/lib/pawn/amx
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE OMP-SDK)

# Force amx.h to use the platform <stdint.h> instead of its legacy typedefs;
# otherwise it collides with the int32_t/uint32_t already provided by sdk.hpp.
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STDINT_H)

# Legacy MapAndreas calls logprintf("...") with string literals into a (char*)
# parameter — GCC/Clang complain, MSVC tolerates. Mute the noise; we don't
# rewrite the legacy core.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(${PROJECT_NAME} PRIVATE -Wno-write-strings -Wno-deprecated-declarations)
endif()

if(MSVC)
	# C4311: legacy MapAndreas_GetAddress casts a 64-bit pointer down to a 32-bit
	# Pawn cell — a known limitation of the native, kept only for source compat.
	target_compile_options(${PROJECT_NAME} PRIVATE /wd4311)
endif()

if(WIN32)
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		NOMINMAX
		WIN32_LEAN_AND_MEAN
		WIN32
		_WIN32
		_CRT_SECURE_NO_WARNINGS
	)
endif()

if(UNIX AND NOT APPLE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE LINUX)
endif()

message(STATUS "MapAndreas: building ${MA_VERSION} for ${MA_ARCH} (pointer size: ${CMAKE_SIZEOF_VOID_P})")
