#ifndef MAPANDREAS_COMMON_HPP
#define MAPANDREAS_COMMON_HPP

// HAVE_STDINT_H is set globally via target_compile_definitions in CMake — we
// can't define it here because pawn.hpp pulls amx.h before this header runs,
// so the gate has to win at TU level.
#include <amx/amx.h>

// Set in openmp_component.cpp::onInit from IPawnComponent::getAmxFunctions().
// Vendored amxplugin.cpp dispatches all amx_* calls through this table.
extern void *pAMXFunctions;

// Mirrors the legacy SA-MP logprintf signature so calls inside MapAndreas.cpp
// and natives.cpp keep compiling. Bound to the open.mp core logger by the
// component before any consumer runs.
typedef void (*logprintf_t)(char *format, ...);
extern logprintf_t logprintf;

#endif
