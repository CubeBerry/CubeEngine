//Author: JEYOON YU
//Project: CubeEngine
//File: GLSkybox.hpp
#pragma once
#include "glew/glew.h"
#include <filesystem>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

class GLTexture;

class GLSkybox
{
public:
    GLSkybox(const std::filesystem::path& path);
    ~GLSkybox();

    void EquirectangularToCube();
    void CalculateIrradiance();
    void PrefilteredEnvironmentMap();
    void BRDFLUT();

	GLuint GetCubeMap() { return equirectangular; };
	GLuint GetIrradiance() { return irradiance; };
	GLuint GetPrefilter() { return prefilter; };
	GLuint GetBRDF() { return brdflut; };
private:
    GLTexture* skyboxTexture;

    GLuint captureFBO, captureRBO;

	// CubeMap converted from Equirectangular
	GLuint equirectangular;
	uint32_t faceSize{ 0 };

	// Irradiance
	GLuint irradiance;
	uint32_t irradianceSize{ 64 };

	// Prefilter
	GLuint prefilter;
	uint32_t baseSize{ 512 };
	uint32_t mipLevels{ 5 };

	// BRDFLUT
	GLuint brdflut;
	uint32_t lutSize{ 512 };

	std::vector<glm::vec3> skyboxVertices = {
	{-1.0f,  1.0f, -1.0f},
	{-1.0f, -1.0f, -1.0f},
	{1.0f, -1.0f, -1.0f },
	{1.0f, -1.0f, -1.0f},
	{1.0f,  1.0f, -1.0f},
	{-1.0f,  1.0f, -1.0f},

	{-1.0f, -1.0f,  1.0f},
	{-1.0f, -1.0f, -1.0f},
	{-1.0f,  1.0f, -1.0f},
	{-1.0f,  1.0f, -1.0f},
	{-1.0f,  1.0f,  1.0f},
	{-1.0f, -1.0f,  1.0f},

	{1.0f, -1.0f, -1.0f},
	{1.0f, -1.0f,  1.0f},
	{1.0f,  1.0f,  1.0f},
	{1.0f,  1.0f,  1.0f},
	{1.0f,  1.0f, -1.0f},
	{1.0f, -1.0f, -1.0f},

	{-1.0f, -1.0f,  1.0f},
	{-1.0f,  1.0f,  1.0f},
	{ 1.0f,  1.0f,  1.0f},
	{ 1.0f,  1.0f,  1.0f},
	{ 1.0f, -1.0f,  1.0f},
	{-1.0f, -1.0f,  1.0f},

	{-1.0f,  1.0f, -1.0f},
	{ 1.0f,  1.0f, -1.0f},
	{ 1.0f,  1.0f,  1.0f},
	{ 1.0f,  1.0f,  1.0f},
	{-1.0f,  1.0f,  1.0f},
	{-1.0f,  1.0f, -1.0f},

	{-1.0f, -1.0f, -1.0f},
	{-1.0f, -1.0f,  1.0f},
	{ 1.0f, -1.0f, -1.0f},
	{ 1.0f, -1.0f, -1.0f},
	{-1.0f, -1.0f,  1.0f},
	{ 1.0f, -1.0f,  1.0f}
	};

	glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	glm::mat4 views[6] = {
	glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
	glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
	glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f,0.f, 1.f)),
	glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f,0.f, -1.f)),
	glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
	glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
	};
};