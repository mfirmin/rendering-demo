# model-viewer
Demo of various advanced 3D rendering techniques, implemented with SDL2 and OpenGL 

Many of these techniques are based on the Advanced Lighting tutorials found here: https://learnopengl.com/

TODO
- [ ] Antialiasing
  - [ ] MSAA (Directly supported by OpenGL)
- [ ] Basic Lighting
  - [ ] Point Lights
  - [ ] Directional Lights
  - [ ] Spotlights
- [ ] Advanced Lighting
  - [ ] Gamma Correction
  - [ ] HDR
  - [ ] Bloom
  - [ ] Deferred Shading
  - [ ] Image-Based Ambient Lighting
      - [ ] Diffuse Irradiance
      - [ ] Specular
- [ ] Shadow Mapping


# Requirements
- CMake (minimum version 3.5): https://cmake.org/install/
- SDL2: https://www.libsdl.org/download-2.0.php
- glew: http://glew.sourceforge.net/

# How to build and run
On linux (not yet tested on macOS), 
```
git clone git@github.com:mfirmin/model-viewer.git
cd model-viewer
mkdir build
cd build
ln -s ../assets .
cmake ..
make
./modelviewer
```

# Usage
TODO