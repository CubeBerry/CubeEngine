//Author: JEYOON YU
//Project: CubeEngine
//File: GLShader.hpp
#include "glew/glew.h"
#include <filesystem>

class GLTexture
{
public:
    GLTexture() = default;
    ~GLTexture();

    void LoadTexture(const std::filesystem::path& path_, std::string name_);
    void UseForSlot(unsigned int texture_unit) const noexcept;
    void SetTextureID(int id) { texID = id; };
    void DeleteTexture();
private:
    GLuint textureHandle{ 0 };
    int width, height;
    int texID;
    std::string name;
};