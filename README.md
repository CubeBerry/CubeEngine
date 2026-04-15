# CubeEngine

[![build](https://github.com/CubeBerry/CubeEngine/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/CubeBerry/CubeEngine/actions/workflows/cmake-single-platform.yml)
![Static Badge](https://img.shields.io/badge/language-C%2B%2B-brightgreen)
![Static Badge](https://img.shields.io/badge/platform-Windows-brightgreen)
![Static Badge](https://img.shields.io/badge/license-MIT-brightgreen)

![Static Badge](https://img.shields.io/badge/api-OpenGL-%235586A4?logo=opengl)
![Static Badge](https://img.shields.io/badge/api-Vulkan-%23A41E22?logo=vulkan)
![Static Badge](https://img.shields.io/badge/api-DirectX_12-limegreen)

CubeEngine is a rendering engine written in C++, developed as a personal hobby project and for portfolio purposes. This engine utilizes an integrated renderer supporting OpenGL, Vulkan and DirectX 12. The primary objective of this project is to implement features learned from college courses, particularly those focused on OpenGL graphics. Furthermore, the project aims to adapt and apply these OpenGL-based techniques to Vulkan, DirectX 12, thereby gaining proficiency in three graphics APIs.

## Minimum Requirements
1. Latest Graphics Driver
2. GPU with Vulkan version 1.4.304.0 or above supported
3. Discrete GPU recommended
4. Download [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) Minimum Version 1.4.304.0
 
## How to Build
### Prerequisites
- **Compiler:** MSVC is currently the only supported compiler.
- **IDE:** Visual Studio 2022 is recommended.
- **Build System:** CMake is required to generate project files.

### Build Instructions
1. Open a terminal (CMD or PowerShell) at the project root directory.
2. Create a build directory and navigate into it:
```dos
mkdir build
cd build
```
3. Generate the project files using CMake. Choose one of the following options:
   - **Standard Build (Recommended):**
    ```dos
    cmake ..
    ```
    - **Enable Mesh Nodes:**

    **Known Issue:** Currently, there is an issue regarding Shader Model 6.9 support. Please refer to https://github.com/CubeBerry/CubeEngine/issues/79 for more details.

    **Note:**  You must enable **Windows Developer Mode** in your system settings to use the Mesh Nodes preview feature.
    ```dos
    cmake .. -D USE_PREVIEW_SDK=ON
    ```
4. Open the generated solution file (`.sln`) and set **"Project"** as the startup project.

## How to Compile Shaders
Shaders can be compiled automatically using the provided Python script or manually using CLI tools.

1. **Automatic (Recommended):**
This script handles the conversion of Slang to SPIR-V/HLSL and HLSL to DXIL automatically.

**Prerequisites:**
* Python 3.x installed.
* Create a `Tools` directory in the project root.
* Download **DirectX Shader Compiler (DXC)** and **Slang**, then extract them into the `Tools` directory.
    * Ensure `dxc.exe` and `slangc.exe` paths match the directory structure defined in `shader_compiler.py`.
    * **Note:** You may need to update `DXC_DIR_NAME` and `SLANG_DIR_NAME` variables in `shader_compiler.py` to match your downloaded folder names.
* Slang can be downloaded from the following [link](https://github.com/shader-slang/slang/releases) or included in the Vulkan SDK since version 1.3.296.0.

**Command:**
```bash
python shader_compiler.py
```

2. **Manual Compilation:**
If you prefer to compile shaders manually, use the commands below.

## How to Compile Slang Shading Language
<details>
<summary><strong>How to Compile Slang Shading Language</strong> (Click to expand)</summary>
Shaders are located in Engine/shaders.

1. From .slang to .glsl
```
GLSL
slangc Skybox.slang -profile glsl_460 -entry vertexMain -stage vertex -target glsl -o Skybox.vert

slangc Skybox.slang -profile glsl_460 -entry fragmentMain -stage fragment -target glsl -o Skybox.frag
```
2. From .slang to .spv
```
SPIR-V
slangc Skybox.slang -profile glsl_460 -entry vertexMain -stage vertex -target spirv -o Skybox.vert.spv

slangc Skybox.slang -profile glsl_460 -entry fragmentMain -stage fragment -target spirv -o Skybox.frag.spv
```
3. From .slang to .hlsl
```
HLSL (Add -D__hlsl__ when converting to HLSL)
slangc Skybox.slang -profile sm_5_1 -entry vertexMain -stage vertex -target hlsl -o Skybox.vert.hlsl -D__hlsl__

slangc Skybox.slang -profile sm_5_1 -entry fragmentMain -stage fragment -target hlsl -o Skybox.frag.hlsl -D__hlsl__
```
</details>

## How to Compile HLSL (High-Level Shader Language) to DXIL (DirectX Intermediate Language)
<details>
<summary><strong>How to Compile HLSL to DXIL</strong> (Click to expand)</summary>

```
// Vertex Shader
dxc 3D.vert.hlsl -T vs_5_1 -E vertexMain -Fo 3D.vert.cso

// Pixel Shader
dxc 3D.frag.hlsl -T ps_5_1 -E fragmentMain -Fo 3D.frag.cso

// Compute Shader
dxc Compute.compute.hlsl -T cs_5_1 -E computeMain -Fo Compute.compute.cso

// Mesh Shader
dxc 3D.mesh.hlsl -T ms_6_5 -E meshMain -Fo 3D.mesh.cso

// Work Graphs Shader
dxc WorkGraphs.hlsl -T lib_6_8 -E broadcastNode -Fo WorkGraphs.cso
```
</details>

## Features Table
**Legend:** ✅: Implemented / ❌: Not Supported / ➖: Not Applicable

| Feature | Engine Core (CPU) | OpenGL | Vulkan | DirectX 12 |
| :--- | :---: | :---: | :---: | :---: |
| **Core Engine** | | | | |
| CPU Dump Writer | ✅ | ➖ | ➖ | ➖ |
| CRT Memory Leak Detector | ✅ | ➖ | ➖ | ➖ |
| Sound Manager (FMOD) | ✅ | ➖ | ➖ | ➖ |
| Physics Manager (2D / 3D) | ✅ | ➖ | ➖ | ➖ |
| Logger (CMD, CSV) | ✅ | ➖ | ➖ | ➖ |
| **Rendering Features** | | | | |
| 2D (Sprite Based Animation) Rendering | ➖ | ✅ | ✅ | ✅ |
| 3D Mesh Forward Rendering | ➖ | ✅ | ✅ | ✅ |
| 3D Mesh Deferred Rendering | ➖ | ❌ | ❌ | ✅ |
| PBR (Physically Based Rendering) | ➖ | ✅ | ✅ | ✅ |
| IBL (Image Based Lighting) | ➖ | ✅ | ✅ | ✅ |
| Moment Shadow Mapping | ➖ | ❌ | ❌ | ✅ |
| Skybox | ➖ | ✅ | ✅ | ✅ |
| Normal Vector Visualization | ➖ | ✅ | ✅ | ✅ |
| [Assimp Model Loading](https://github.com/assimp/assimp) | ✅ | ➖ | ➖ | ➖ |
| Skeletal Animation | ➖ | ✅ | ✅ | ✅ |
| **Graphics Backend Features** | | | | |
| [Slang Shading Language](https://shader-slang.org/) | ➖ | ❌ | ✅ | ✅ |
| [ImGui](https://github.com/ocornut/imgui) | ➖ | ✅ | ✅ | ✅ |
| [NVIDIA Nsight Aftermath](https://developer.nvidia.com/nsight-aftermath) | ➖ | ❌ | ❌ | ✅ |
| Compute Shader | ➖ | ❌ | ❌ | ✅ |
| Mesh Shader | ➖ | ❌ | ❌ | ✅ |
| Work Graphs | ➖ | ❌ | ❌ | ✅ |
| **Optimization Techniques** | | | | |
| MSAA | ➖ | ✅ | ✅ | ✅ |
| Vertex Position Quantization (2D: vec2 -> uint (16 bit, 16 bit), 3D: vec3 -> uint (11 bit, 11 bit, 10 bit)) | ✅ | ➖ | ➖ | ➖ |
| [AMD FidelityFX™ Contrast Adaptive Sharpening](https://gpuopen.com/fidelityfx-cas/) | ➖ | ❌ | ❌ | ✅ |
| [AMD FidelityFX™ Super Resolution 1](https://gpuopen.com/fidelityfx-superresolution/) | ➖ | ❌ | ❌ | ✅ |

## Screenshots
<img width="1920" height="1032" alt="image" src="https://github.com/user-attachments/assets/1b31227f-5470-480f-a855-da9927f53dab" />

![image](https://github.com/user-attachments/assets/4b74e0cf-c652-4af2-81ed-981027c0b5b2)

![image](https://github.com/user-attachments/assets/36c003b4-65c5-413f-9faf-e71366a791f3)

![image](https://github.com/user-attachments/assets/4a713821-5f2c-4fe5-81c4-fba9a72dfd67)

<img width="2560" height="1392" alt="image" src="https://github.com/user-attachments/assets/7b40822c-f84d-4f42-ae0a-7a203dc34732" />

<img width="1277" height="665" alt="Image" src="https://github.com/user-attachments/assets/b5840bfc-4075-4dda-92ad-89d2b37f0b0d" />

![image](https://github.com/user-attachments/assets/e7221576-5010-48b5-8a58-78bd5f197625)

## Future Plans
1. CPU Profiler, Memory Debugger
2. Deferred Shading
3. SSAO
4. Normal / Parallax Mapping
5. C++ Code Optimization

## License
Licensed under the [MIT License](https://github.com/minjae-yu/CubeEngine/blob/main/LICENSE).

Music by [DavidKBD](https://www.davidkbd.com/) under the [CC BY 4.0 License](https://creativecommons.org/licenses/by/4.0/).

Music by [Abstraction](https://abstractionmusic.com/) under the [CC0 1.0 Universal License](https://creativecommons.org/publicdomain/zero/1.0/).

Asset by [Pupkin](https://trevor-pupkin.itch.io/tech-dungeon-roguelite)

Sound Effect by [効果音ラボ](https://soundeffect-lab.info/)
