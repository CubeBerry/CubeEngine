//Author: JEYOON YU
//Project: CubeEngine
//File: GLTexture.cpp
#include "GLTexture.hpp"
#include "stb-master/stb_image.h"

GLTexture::~GLTexture()
{
	DeleteTexture();
}

void GLTexture::LoadTexture(const std::filesystem::path& path_, std::string name_)
{
	stbi_set_flip_vertically_on_load(true);
	int color;
	//STBI_rgb_alpha == 4
	unsigned char* data = stbi_load(path_.string().c_str(), &width, &height, &color, STBI_rgb_alpha);
	
	//DeleteTexture();

	glCreateTextures(GL_TEXTURE_2D, 1, &textureHandle);
	// Create immutable storage of widthxheight RGBA8 GPU memory with only one texture level
	constexpr GLsizei ONE_TEXTURE_LEVEL = 1;
	glTextureStorage2D(textureHandle, ONE_TEXTURE_LEVEL, GL_RGBA8, width, height);
	// Send `colors` data to GPU memory
	constexpr GLint   FIRST_LEVEL = 0;
	constexpr GLsizei OFFSET_X = 0, OFFSET_Y = 0;
	glTextureSubImage2D(textureHandle, FIRST_LEVEL, OFFSET_X, OFFSET_Y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<unsigned int*>(data));

	//Set Filtering
	// invoke glTextureParameteri to set minification filter
	glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// invoke glTextureParameteri to set magnification filter
	glTextureParameteri(textureHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//Set Wrapping
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	name = name_;
}

void GLTexture::UseForSlot(unsigned int unit) const noexcept
{
	// == Shader layout binding
	// invoke glBindTextureUnit to associate the texture unit with this texture
	glBindTextureUnit(unit, textureHandle);
}

void GLTexture::DeleteTexture()
{
	glDeleteTextures(1, &textureHandle);
	textureHandle = 0;
	width = 0;
	height = 0;
}
