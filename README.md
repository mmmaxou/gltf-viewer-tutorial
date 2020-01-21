# glTF Viewer Tutorial

This is the SDK for my glTF Viewer tutorial https://celeborn2bealive.github.io/openglnoel/docs/gltf-viewer-01-intro-01-intro

## Build process commands

Build using CMake

```bash
mkdir build-gltf-viewer-tutorial
cd build-gltf-viewer-tutorial
cmake ../gltf-viewer-tutorial-git
```

Make project 

```bash
make -j
```

In one command

```bash
cmake ../gltf-viewer-tutorial-git && make -j
```

## Test program using Model DamagedHelmet

Be in the build folder

```bash
./bin/gltf-viewer viewer ../glTF-Sample-Models/DamagedHelmet/glTF/DamagedHelmet.gltf 
```

Or

```bash
make -j && ./bin/gltf-viewer viewer ../glTF-Sample-Models/DamagedHelmet/glTF/DamagedHelmet.gltf
make -j && ./bin/gltf-viewer viewer ../glTF-Sample-Models/Sponza/glTF/Sponza.gltf
```