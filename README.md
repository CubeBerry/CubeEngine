# CubeEngine

![Static Badge](https://img.shields.io/badge/language-C%2B%2B-brightgreen)
![Static Badge](https://img.shields.io/badge/platform-Windows-brightgreen)
![Static Badge](https://img.shields.io/badge/license-MIT-brightgreen)

![Static Badge](https://img.shields.io/badge/api-OpenGL-%235586A4?logo=opengl)
![Static Badge](https://img.shields.io/badge/api-Vulkan-%23A41E22?logo=vulkan)
![Static Badge](https://img.shields.io/badge/api-DirectX_12-limegreen)

CubeEngine is a rendering engine written in C++, developed as a personal hobby project and for portfolio purposes. This engine utilizes an integrated renderer based on both OpenGL, Vulkan and DirectX 12. The primary objective of this project is to implement features learned from college courses, particularly those focused on OpenGL graphics. Furthermore, the project aims to adapt and apply these OpenGL-based techniques to Vulkan, DirectX 12, thereby gaining proficiency in three graphics APIs.

## Minimum Requirements
1. Latest Graphics Driver
2. GPU with Vulkan version 1.4.304.0 or above supported
3. Discrete GPU recommended
4. Download [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) Minimum Version 1.4.304.0
 
## How to Build
Currently it only supports MSVC compiler(Visual Studio 2022 Recommended).
1. Open CMD at a root directory.
2. Input "mkdir build" -> "cd build" -> "cmake .."
3. Set "Project" project as a startup project

All required APIs and Libraries are integrated in Project.

## How to Compile Slang Shading Language
To compile Slang Shading Language, slangc is required. slangc is able to download from the following [link](https://github.com/shader-slang/slang/releases) or included in the Vulkan SDK since version 1.3.296.0.

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

## Features
1. Running on OpenGL & Vulkan & DirectX 12 Graphics APIs.
2. [Slang Shading Language](https://shader-slang.org/)
3. [ImGui](https://github.com/ocornut/imgui) Integrated
4. CPU Dump Writer, CRT Memory Leak Detector, [NVIDIA Nsight Aftermath](https://developer.nvidia.com/nsight-aftermath) (DX12 Only)
5. SoundManager with ImGui UI
6. Basic 2D Game Engine Features
7. 3D Plane, Cube(3D Collision Applied), Sphere, Tours, Cylinder, Cone Meshes + Normal Vector Debugger
8. Assimp Model Loading
9. Blinn-Phong Lighting, Physically Based Rendering
10. PBR Image Based Lighting
11. Skybox
12. MSAA

## Screenshots
![image](https://github.com/user-attachments/assets/4b74e0cf-c652-4af2-81ed-981027c0b5b2)

![image](https://github.com/user-attachments/assets/36c003b4-65c5-413f-9faf-e71366a791f3)

![image](https://github.com/user-attachments/assets/4a713821-5f2c-4fe5-81c4-fba9a72dfd67)

![image](https://github.com/user-attachments/assets/0116de70-4fcc-465d-be30-21321d67ee25)

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
