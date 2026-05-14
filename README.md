# Renderer

A high-performance procedural terrain renderer with real-time LOD, vegetation instancing, and culling techniques.

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [System Requirements](#system-requirements)
- [Usage Guide](#usage-guide)
- [Controls](#controls)
- [Configuration](#configuration)
- [Performance Metrics](#performance-metrics)
- [Project Architecture](#project-architecture)
- [Results & Documentation](#results--documentation)
---

## Overview

**Full Documentation:** See [Documentation.pdf](./Documentation.pdf) for comprehensive technical details, algorithms, and research findings.

A professional-grade OpenGL terrain rendering engine for Windows, designed for real-time terrain visualization and experimentation. The system combines multiple advanced rendering techniques to deliver high-fidelity procedural terrain with interactive camera controls and a comprehensive ImGui-based configuration panel.

**Key Capabilities:**
- Dynamic terrain generation using noise algorithms
- Hierarchical level-of-detail (LOD) patch-based rendering
- Advanced spatial optimization through frustum and backface culling
- Instanced vegetation rendering with multiple tree types
- Real-time performance profiling and analysis
- Interactive parameter tuning and live preview

---

## Key Features

### Terrain Rendering
- **Procedural Generation**: Perlin noise-based fBm heightmap creation with configurable parameters
- **Level-of-Detail System**: Patch-based LOD mechanism for optimal performance across all camera distances
- **Dynamic Mesh Generation**: Real-time mesh tessellation and updating

### Vegetation System
- **Multi-type Generation**: Support for multiple tree species and vegetation models
- **Instancing Support**: GPU-accelerated instanced rendering for vegetation entities
- **Shadow Rendering**: Shadow mapping for vegetation elements
- **Configurable Placement**: Real-time vegetation density and distribution parameters

### Optimization & Culling
- **Frustum Culling**: Efficient viewport-based object frustration culling
- **Backface Culling**: Back-facing polygon elimination for improved performance
- **Spatial Acceleration**: Optimized data structures for fast culling operations

### Visual Enhancement
- **Customizable Skybox**: Dynamic sky rendering with adjustable parameters
- **Atmospheric Effects**: Configurable fog system for depth perception
- **Advanced Lighting**: Animated sun with time-based lighting simulation
- **Flexible Shading Models**: Multiple shading algorithms to choose from

### Profiling & Analysis
- **On-Screen Profiler**: Real-time performance metrics display
- **Metrics Export**: CSV export functionality for offline analysis
- **Frame Timing**: Detailed frame time measurements and statistics
- **Performance Visualization**: Graph-based performance metric charts

---

## System Requirements

| Requirement | Specification |
|---|---|
| **Operating System** | Windows 10 or newer |
| **IDE** | Visual Studio 2022 with C++ desktop workload |
| **Architecture** | x64 (x32 supported but not recommended) |
| **GPU** | OpenGL 4.5+ compatible graphics card |
| **VRAM** | Minimum 2GB recommended |
| **RAM** | Minimum 4GB |

**Third-party libraries** (included in repository):
- ImGui - Immediate-mode GUI
- GLEW - OpenGL Extension Wrangler
- FreeGLUT - OpenGL windowing toolkit
- GLM - Mathematics library
- SOIL - Simple OpenGL Image Library
- GLAD - OpenGL loader

---

## Usage Guide

### First Launch
1. The application opens with a 3D terrain scene and an ImGui control panel
2. Use the control panel to adjust terrain, noise, lighting, and rendering parameters
3. Changes apply in real-time with optional scene reinitialization

### Live Parameter Tuning
- Adjust terrain noise parameters to regenerate heightmaps
- Modify LOD distances for rendering optimization
- Toggle culling modes to observe performance impacts
- Experiment with vegetation density and distribution

### Data Export
- Performance metrics are automatically exported to `profiler_metrics.csv` on shutdown
- Use provided Python scripts to visualize profiler data as charts

---

## Controls

| Input | Action |
|---|---|
| **W, A, S, D** | Move camera forward/left/back/right |
| **Arrow Keys** | Rotate camera (spherical mode) |
| **Right Mouse Drag** | Manual camera rotation |
| **+ / -** | Increase/decrease camera distance |
| **Q** | Toggle wireframe rendering mode |
| **E** | Restore filled polygon rendering |
| **O** | Toggle overview/orthographic camera mode |
| **Esc** | Exit application |

---

## Configuration

### ImGui Control Panel
The integrated ImGui interface provides real-time control over:

**Terrain Settings:**
- Noise scale, frequency, and octaves
- Terrain height multiplier and offset
- Patch LOD distances and thresholds

**Rendering Options:**
- Frustum culling enable/disable
- Backface culling toggle
- Wireframe mode
- Shading model selection

**Vegetation:**
- Tree density and distribution
- Instancing enable/disable
- Shadow rendering options

**Lighting & Atmosphere:**
- Sun rotation and intensity
- Fog distance and density
- Skybox selection

---

## Performance Metrics

The application includes a comprehensive profiling system that tracks:

- **Frame Time**: Total time per frame (ms)
- **Draw Calls**: Number of GPU draw commands per frame
- **Vertices Rendered**: Vertex count per frame
- **Culled Objects**: Frustum/backface culled object counts
- **Memory Usage**: GPU and CPU memory statistics

### Accessing Performance Data
1. Run the application and observe on-screen profiler
2. Close the application to auto-export metrics
3. Open `profiler_metrics.csv` to inspect raw data
4. Use `plot_profiler_metrics.py` to generate visualization charts

---

## Project Architecture

### Core Components

```
Renderer/
├── main.cpp                    # Application entry point and render loop
├── sources/                    # Core engine systems
│   ├── Camera.{cpp,h}         # Camera system and navigation
│   ├── InputManager.{cpp,h}   # Input handling and event processing
│   ├── Terrain.{cpp,h}        # Terrain generation and rendering
│   ├── Shader.{cpp,h}         # Shader management and compilation
│   ├── LightManager.{cpp,h}   # Lighting system
│   ├── VegetationManager.{cpp,h} # Vegetation generation and instancing
│   ├── ModelManager.{cpp,h}   # 3D model management
│   ├── Profiler.{cpp,h}       # Performance profiling and metrics
│   ├── MaterialManager.{cpp,h}# Material and texture management
│   ├── ShaderManager.{cpp,h}  # Shader compilation and caching
│   └── models/                # Primitive and scene models
├── shaders/                    # GLSL shader programs
│   ├── Terrain*.vert/frag     # Terrain rendering shaders
│   ├── Vegetation*.vert/frag  # Vegetation rendering shaders
│   ├── Light*.vert/frag       # Lighting shaders
│   ├── Instancing*.vert/frag  # Instancing shaders
│   └── ...
├── include/                    # External headers and libraries
└── lib/                        # Platform-specific binary dependencies
```

### Data Flow
1. **Input** → InputManager processes keyboard/mouse events
2. **Camera** → Camera system updates view/projection matrices
3. **Terrain** → Procedural generation and LOD computation
4. **Culling** → Frustum and backface culling
5. **Rendering** → Shader-based rendering with instancing
6. **Profiling** → Metrics collection and export

---

## Results & Documentation

### Performance Analysis
Detailed performance metrics and optimization results are available in:
- `profiler_metrics.csv` - Raw profiler data
- `charts/` - Generated visualization charts
- [Documentation.pdf](./Documentation.pdf) - Research and analysis

### Performance Visualizations

#### Optimized Run Performance
![Optimized Run Performance Chart](./Renderer/charts/Optimized%20Run.png)

This chart shows the performance metrics of the optimized renderer configuration, demonstrating:
- Consistent frame times across varying scene complexity
- Effective load distribution with LOD and culling
- Peak performance with all optimizations enabled

### Additional Performance Charts
Other performance analysis charts available in `charts/` directory:
- **LODs.png** - Level of detail impact on performance
- **Culling.png** - Frustum and backface culling effectiveness
- **Instancing.png** - Vegetation instancing performance gains
- **Illumination.png** - Lighting system performance analysis

---

<div align="center">

**Built with C++ and OpenGL**

</div>
