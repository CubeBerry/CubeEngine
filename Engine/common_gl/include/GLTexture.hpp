//Author: JEYOON YU
//Project: CubeEngine
//File: GLShader.hpp
#pragma once
#include "glew/glew.h"
#include <filesystem>
#include <glm/vec2.hpp>

class GLTexture
{
public:
    GLTexture() = default;
    ~GLTexture();

    void LoadTexture(const std::filesystem::path& path_, std::string name_, int id);
    void SetTextureID(int id) { texID = id; };
    void DeleteTexture();

    glm::vec2 GetSize() const { return glm::vec2(width, height); };
    std::string GetName() const { return name; };
    int GetTextrueId() { return texID; }
private:
    void UseForSlot(unsigned int texture_unit) const noexcept;

    GLuint textureHandle{ 0 };
    int width, height;
    int texID;
    std::string name;
};