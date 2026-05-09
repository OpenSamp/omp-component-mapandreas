// Open.mp component entry point for the MapAndreas port.
//
// The component plays two roles:
//   1. Drives the open.mp lifecycle (onLoad/onInit/free) and registers the
//      legacy MapAndreas_* AMX natives via IPawnComponent's event dispatcher,
//      so existing Pawn gamemodes keep working unchanged.
//   2. Exposes IMapAndreasComponent (see mapandreas-api.hpp) so other
//      components can query the height-map without going through Pawn.
//
// The legacy height-map core (CMapAndreas in MapAndreas.cpp) is untouched —
// we just delegate the IExtension methods to the global instance.

#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>

#include "MapAndreas.hpp"
#include "common.hpp"
#include "mapandreas-api.hpp"
#include "natives.hpp"
#include "openmp_component.h"

logprintf_t logprintf = nullptr;

extern CMapAndreas MapAndreas;

namespace
{
	ICore *g_core = nullptr;
	IPawnComponent *g_pawn = nullptr;

	AMX_NATIVE_INFO g_mapAndreasNatives[] = {
		{ "MapAndreas_Init", Natives::Init },
		{ "MapAndreas_FindZ_For2DCoord", Natives::FindZ_For2DCoord },
		{ "MapAndreas_FindAverageZ", Natives::FindAverageZ },
		{ "MapAndreas_Unload", Natives::_Unload },
		{ "MapAndreas_SetZ_For2DCoord", Natives::SetZ_For2DCoord },
		{ "MapAndreas_SaveCurrentHMap", Natives::SaveCurrentHMap },
		{ "MapAndreas_GetAddress", Natives::GetAddress },
		{ nullptr, nullptr }
	};

	// Init() takes a writable buffer (legacy non-const char*). Stash StringView
	// payloads in a thread_local so we don't malloc per call.
	thread_local char g_pathBuffer[1024];

	char *toMutable(StringView sv)
	{
		auto len = sv.length();
		if (len >= sizeof(g_pathBuffer)) len = sizeof(g_pathBuffer) - 1;
		std::memcpy(g_pathBuffer, sv.data(), len);
		g_pathBuffer[len] = '\0';
		return g_pathBuffer;
	}

	class MapAndreasComponent final
		: public IComponent
		, public IMapAndreasComponent
		, public PawnEventHandler
	{
	public:
		PROVIDE_UID(kMapAndreasComponentUID)

		StringView componentName() const override { return "MapAndreas"; }
		SemanticVersion componentVersion() const override { return SemanticVersion(1, 2, 0, 0); }

		// ---- IComponent / IExtensible -----------------------------------------------
		IExtension *getExtension(UID id) override
		{
			if (id == IMapAndreasComponent::ExtensionIID)
				return static_cast<IMapAndreasComponent *>(this);
			return IExtensible::getExtension(id);
		}

		void onLoad(ICore *c) override
		{
			g_core = c;
			logprintf = &ma_logprintf;

			ma_logprintf((char *)"*********************");
			ma_logprintf((char *)"** MapAndreas      **");
			ma_logprintf((char *)"** open.mp port    **");
			ma_logprintf((char *)"*********************");
		}

		void onInit(IComponentList *list) override
		{
			g_pawn = list->queryComponent<IPawnComponent>();
			if (g_pawn)
			{
				pAMXFunctions = const_cast<void *>(static_cast<const void *>(g_pawn->getAmxFunctions().data()));
				g_pawn->getEventDispatcher().addEventHandler(this);
			}
			else if (g_core)
			{
				g_core->logLn(LogLevel::Error, "MapAndreas: Pawn component not available; MapAndreas_* natives will not be registered");
			}
		}

		void onReady() override { }
		void onFree(IComponent * /*component*/) override { }

		void free() override
		{
			if (g_pawn)
			{
				g_pawn->getEventDispatcher().removeEventHandler(this);
				g_pawn = nullptr;
			}
			pAMXFunctions = nullptr;
			g_core = nullptr;
		}

		void reset() override { }

		// ---- PawnEventHandler --------------------------------------------------------
		void onAmxLoad(IPawnScript &script) override
		{
			if (AMX *amx = script.GetAMX())
				amx_Register(amx, g_mapAndreasNatives, -1);
		}

		void onAmxUnload(IPawnScript & /*script*/) override { }

		// ---- IMapAndreasComponent ----------------------------------------------------
		int init(int modeArg, StringView path) override
		{
			char *buf = path.empty() ? (char *)"" : toMutable(path);
			int len = path.empty() ? 0 : static_cast<int>(path.length()) + 1;
			return MapAndreas.Init(modeArg, buf, len);
		}

		bool isInitialized() const override { return MapAndreas.GetMode() != 0; }
		int mode() const override { return MapAndreas.GetMode(); }

		bool findZ(float x, float y, float &outZ) override
		{
			outZ = MapAndreas.FindZ_For2DCoord(x, y);
			return outZ > 0.0f;
		}

		bool findAverageZ(float x, float y, float &outZ) override
		{
			outZ = MapAndreas.GetAverageZ(x, y);
			return outZ > 0.0f;
		}

		bool setZ(float x, float y, float z) override
		{
			return MapAndreas.SetZ_For2DCoord(x, y, z) != 0;
		}

		bool saveCurrentHMap(StringView path) override
		{
			if (path.empty()) return false;
			return MapAndreas.SaveCurrentHMap(toMutable(path)) != 0;
		}

		bool unload() override { return MapAndreas.Unload(); }
	};

	MapAndreasComponent g_mapAndreasComponent;
}

extern "C" void ma_logprintf(char *format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	buffer[sizeof(buffer) - 1] = '\0';
	va_end(args);

	if (g_core)
	{
		g_core->printLn("%s", buffer);
	}
	else
	{
		std::fprintf(stdout, "%s\n", buffer);
	}
}

COMPONENT_ENTRY_POINT()
{
	return &g_mapAndreasComponent;
}
