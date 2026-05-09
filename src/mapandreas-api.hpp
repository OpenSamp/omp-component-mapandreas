// Public C++ interface exposed by mapandreas as an open.mp IExtension.
//
// Other components query this via:
//
//     IComponent* maComp = components->queryComponent(kMapAndreasComponentUID);
//     IMapAndreasComponent* ma = queryExtension<IMapAndreasComponent>(maComp);
//
// This is the canonical open.mp idiom for cross-component APIs (mirrors
// IStreamerComponent in omp-streamer-component, etc.). Wrapper repos that
// bridge to non-C++ runtimes (e.g. SampSharp.OpenMp.MapAndreas) maintain a
// parallel mirror of this header.

#pragma once

#include <component.hpp>

constexpr UID kMapAndreasComponentUID = UID(0x4d6170416e647273ULL); // "MapAndrs"
constexpr UID kMapAndreasExtensionUID = UID(0x4d6170416e644558ULL); // "MapAndEX"

struct IMapAndreasComponent : public IExtension
{
	PROVIDE_EXT_UID(kMapAndreasExtensionUID)

	// MAP_ANDREAS_MODE_* — kept as plain int to mirror the legacy Pawn API.
	// 0=NONE, 1=MINIMAL, 3=FULL, 4=NOBUFFER. (2=MEDIUM is reserved upstream.)

	/// Loads a height-map. Returns MAP_ANDREAS_ERROR_* (0 = success). Empty
	/// path uses the mode's default file (scriptfiles/SAfull.hmap or SAmin.hmap).
	virtual int init(int mode, StringView path) = 0;

	virtual bool isInitialized() const = 0;
	virtual int mode() const = 0;

	/// Returns true on hit (outZ populated), false on miss / out-of-bounds.
	virtual bool findZ(float x, float y, float& outZ) = 0;
	virtual bool findAverageZ(float x, float y, float& outZ) = 0;

	virtual bool setZ(float x, float y, float z) = 0;
	virtual bool saveCurrentHMap(StringView path) = 0;
	virtual bool unload() = 0;

	void reset() override { }
};
