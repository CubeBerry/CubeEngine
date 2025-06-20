# CubeEngine

![Static Badge](https://img.shields.io/badge/language-C%2B%2B-brightgreen)
![Static Badge](https://img.shields.io/badge/platform-Windows-brightgreen)
![Static Badge](https://img.shields.io/badge/license-MIT-brightgreen)

CubeEngine is a rendering engine written in C++, developed as a personal hobby project and for portfolio purposes. This engine utilizes an integrated renderer based on both OpenGL and Vulkan. The primary objective of this project is to implement features learned from college courses, particularly those focused on OpenGL graphics. Furthermore, the project aims to adapt and apply these OpenGL-based techniques to Vulkan, thereby gaining proficiency in both graphics APIs.

## Minimum Requirements
1. Latest Graphics Driver
2. GPU with Vulkan version 1.3.296.0 or above supported
3. Discrete GPU recommended
4. Download [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) Minimum Version 1.3.296.0
 
## How to Build
Currently it only supports MSVC compiler(Visual Studio 2022 Recommended).
1. Open CMD at a root directory.
2. Input "mkdir build" -> "cd build" -> "cmake .."
3. Set "Project" project as a startup project

All required APIs and Libraries are integrated in Project.

## Features
1. Running on OpenGL & Vulkan Graphics APIs.
2. [Slang Shading Language](https://shader-slang.org/)
3. [ImGui](https://github.com/ocornut/imgui) Integrated
4. SoundManager with ImGui UI
5. Basic 2D Game Engine Features
6. 3D Plane, Cube(3D Collision Applied), Sphere, Tours, Cylinder, Cone Meshes + Normal Debugger
7. Assimp Model Loading
8. Blinn-Phong Lighting, Physically Based Rendering
9. PBR Image Based Lighting
10. Skybox
11. MSAA

## Screenshots
![image](https://github.com/user-attachments/assets/4b74e0cf-c652-4af2-81ed-981027c0b5b2)

![image](https://github.com/user-attachments/assets/36c003b4-65c5-413f-9faf-e71366a791f3)

![image](https://github.com/user-attachments/assets/4a713821-5f2c-4fe5-81c4-fba9a72dfd67)

![image](https://github.com/user-attachments/assets/0116de70-4fcc-465d-be30-21321d67ee25)

![image](https://github.com/user-attachments/assets/e7221576-5010-48b5-8a58-78bd5f197625)

## Future Plans
1. DirectX12
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
