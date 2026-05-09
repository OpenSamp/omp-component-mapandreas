# omp-component-mapandreas

Open.mp port of the legacy SA-MP MapAndreas plugin (originally by Kalcor, extended by Mauzen). Loads a pre-baked GTA: SA height map (`SAfull.hmap` / `SAmin.hmap`) and answers ground-Z queries — handy for anti-cheat (airbreak detection), spawn placement, "fall-through-the-ground" prevention, and similar.

The repo wraps the original height-map core in an open.mp `IComponent`, so it loads side-by-side with other components instead of via the legacy `Load`/`AmxLoad` plugin entry points.

## What's exposed

Two parallel surfaces, both backed by the same singleton `CMapAndreas`:

**Pawn natives** (kept for source-compat with legacy gamemodes — `mapandreas.inc`):
- `MapAndreas_Init(mode, const name[]="", len=sizeof(name))`
- `MapAndreas_FindZ_For2DCoord(Float:X, Float:Y, &Float:Z)`
- `MapAndreas_FindAverageZ(Float:X, Float:Y, &Float:Z)`
- `MapAndreas_SetZ_For2DCoord(Float:X, Float:Y, Float:Z)`
- `MapAndreas_SaveCurrentHMap(const name[])`
- `MapAndreas_Unload()`
- `MapAndreas_GetAddress()` — only meaningful on x86 (returns truncated pointer on x64; left for legacy compat)

**C++ component interface** — `IMapAndreasComponent : IExtension` declared in [src/mapandreas-api.hpp](src/mapandreas-api.hpp). Other open.mp components query it the canonical way:

```cpp
#include "mapandreas-api.hpp"

IComponent* maComp = components->queryComponent(kMapAndreasComponentUID);
IMapAndreasComponent* ma = queryExtension<IMapAndreasComponent>(maComp);
if (ma) {
    float z;
    if (ma->findZ(x, y, z)) { /* ... */ }
}
```

For non-C++ runtimes (e.g. SampSharp / .NET), see [openmp-sampsharp-mapandreas](https://github.com/OpenSamp/SampSharp.OpenMp.MapAndreas) — it ships a separate `SampSharp.MapAndreas` component that queries this extension and re-exports a flat C ABI suitable for P/Invoke.

## Building

Standard CMake. Defaults to host bitness (x64); pass `-DMA_ARCH=x86` to force 32-bit.

```bash
git clone --recursive https://github.com/OpenSamp/omp-component-mapandreas.git
cd omp-component-mapandreas
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --config RelWithDebInfo
# → build/bin/mapandreas.{so,dll}
```

## CI / artifacts

GitHub Actions ([build-and-publish.yml](.github/workflows/build-and-publish.yml)) builds two flavours on every push:

- **Linux x64** → published to GHCR as a scratch image: `ghcr.io/opensamp/omp-component-mapandreas:<tag>` containing `mapandreas.so` at root. Consumed via `COPY --from=ghcr.io/opensamp/omp-component-mapandreas:latest /mapandreas.so /target/`.
- **Windows x64** → uploaded as a workflow artifact (`mapandreas-windows-x64`) containing `mapandreas.dll` + `.pdb`. Download from the run page.

## Runtime

Drop the binary into your open.mp server's `components/` folder. The component registers automatically on `omp-server` startup; no extra config.lua entries needed beyond the standard component discovery.

Height-map data files (`SAfull.hmap` ≈ 72MB / `SAmin.hmap` ≈ 8MB) are not shipped here — grab them from the original SA-MP MapAndreas distribution and drop into your server's `scriptfiles/`. `MapAndreas_Init(mode)` defaults to `scriptfiles/SAfull.hmap` for `MODE_FULL`/`MODE_NOBUFFER` and `scriptfiles/SAmin.hmap` for `MODE_MINIMAL`.

## Modes

| Mode      | Memory      | Disk I/O per query | Resolution |
|-----------|-------------|--------------------|------------|
| `FULL`    | ~72 MB      | none               | 1m grid    |
| `MINIMAL` | ~8 MB       | none               | 3m grid    |
| `NOBUFFER`| <1 MB       | seek+read each call| 1m grid    |
| `MEDIUM`  | reserved by upstream — never implemented |

## License

Original plugin: SA-MP team, public domain. open.mp port: same.
