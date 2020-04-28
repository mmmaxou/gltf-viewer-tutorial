# glTF Viewer Tutorial

This is the SDK for my glTF Viewer tutorial <https://celeborn2bealive.github.io/openglnoel/docs/gltf-viewer-01-intro-01-intro>

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

You must be in the build folder

### DamagedHelmet Default

```bash
make -j && ./bin/gltf-viewer viewer ../glTF-Sample-Models/DamagedHelmet/glTF/DamagedHelmet.gltf
```

### DamagedHelmet Custom lookat

```bash
make -j && ./bin/gltf-viewer viewer ../glTF-Sample-Models/DamagedHelmet/glTF/DamagedHelmet.gltf --lookat "2.49055,-0.439571,2.37761,1.79665,-0.359656,1.66199,0.0556313,0.996802,0.0573718"
```

### DamagedHelmet Image Output

```bash
make -j && ./bin/gltf-viewer viewer ../glTF-Sample-Models/DamagedHelmet/glTF/DamagedHelmet.gltf --lookat "2.49055,-0.439571,2.37761,1.79665,-0.359656,1.66199,0.0556313,0.996802,0.0573718" --output helmet_output.png
```

## Test program using Model Sponza

You must be in the build folder

### Sponza Default

```bash
make -j && ./bin/gltf-viewer viewer ../glTF-Sample-Models/Sponza/glTF/Sponza.gltf
```

### Sponza Custom lookat

```bash
make -j && ./bin/gltf-viewer viewer ../glTF-Sample-Models/Sponza/glTF/Sponza.gltf --lookat "-5.26056,6.59932,0.85661,-4.40144,6.23486,0.497347,0.342113,0.931131,-0.126476"
```

### Sponza Image Output

```bash
make -j && ./bin/gltf-viewer viewer ../glTF-Sample-Models/Sponza/glTF/Sponza.gltf --lookat "-5.26056,6.59932,0.85661,-4.40144,6.23486,0.497347,0.342113,0.931131,-0.126476" --output sponza_output.png
```
