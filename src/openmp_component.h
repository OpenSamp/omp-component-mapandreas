// Open.mp component bridge for MapAndreas.
//
// Mirrors the ColAndreas port: an IComponent subclass owns the open.mp
// lifecycle (onLoad/onInit/free) and registers the legacy MapAndreas_* natives
// against every AMX the server loads. The height-map core in MapAndreas.cpp
// is untouched and continues to operate on AMX cells, so the legacy
// mapandreas.inc API stays source-compatible.

#ifndef OPENMP_COMPONENT_H
#define OPENMP_COMPONENT_H

struct ICore;
struct IPawnComponent;

namespace MapAndreasRuntime
{
	ICore *core();
	IPawnComponent *pawn();
}

// Routed to open.mp's core logger by the component. Matches the legacy
// `typedef void(*logprintf_t)(char*, ...)` so MapAndreas/natives keep compiling.
extern "C" void ma_logprintf(char *format, ...);

#endif
