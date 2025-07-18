//Author: JEYOON YU
//Project: CubeEngine
//File: GLSkybox.cpp
#include "GLSkybox.hpp"
#include "GLTexture.hpp"

#include "glCheck.hpp"
#include "GLShader.hpp"
#include "GLVertexArray.hpp"

GLSkybox::GLSkybox(const std::filesystem::path& path)
{
	skyboxTexture = new GLTexture();
	skyboxTexture->LoadTexture(true, path, "skybox", true, 0);
	faceSize = static_cast<uint32_t>(skyboxTexture->GetSize().y);

	glCheck(glCreateFramebuffers(1, &captureFBO));
	glCheck(glCreateRenderbuffers(1, &captureRBO));
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, captureFBO));
	glCheck(glBindRenderbuffer(GL_RENDERBUFFER, captureRBO));
	glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, faceSize, faceSize));
	glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO));

	EquirectangularToCube();
	CalculateIrradiance();
	PrefilteredEnvironmentMap();
	BRDFLUT();
}

GLSkybox::~GLSkybox()
{
	delete skyboxTexture;

	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteTextures(1, &equirectangular);
}

void GLSkybox::EquirectangularToCube()
{
	glCheck(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &equirectangular));
	glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, equirectangular));
	for (GLuint i = 0; i < 6; ++i)
	{
		glCheck(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, faceSize, faceSize, 0, GL_RGBA, GL_FLOAT, nullptr));
	}

	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLShader shaderIBL;
	shaderIBL.LoadShader({ { GLShader::VERTEX, "../Engine/shaders/glsl/Cubemap.vert" }, { GLShader::FRAGMENT, "../Engine/shaders/glsl/Equirectangular.frag" } });
	shaderIBL.Use(true);

	glCheck(glUniform1i(glGetUniformLocation(shaderIBL.GetProgramHandle(), "equirectangularMap"), 0));
	glCheck(glUniformMatrix4fv(glGetUniformLocation(shaderIBL.GetProgramHandle(), "projection"), 1, GL_FALSE, &projection[0][0]));

	glCheck(glActiveTexture(GL_TEXTURE0));
	glCheck(glBindTexture(GL_TEXTURE_2D, skyboxTexture->GetTextureHandle()));

	GLVertexArray vertexArray;
	vertexArray.Initialize();

	GLVertexBuffer vertexBuffer;
	vertexBuffer.SetData(sizeof(glm::vec3) * static_cast<GLsizei>(skyboxVertices.size()), skyboxVertices.data());

	GLAttributeLayout position_layout;
	position_layout.component_type = GLAttributeLayout::Float;
	position_layout.component_dimension = GLAttributeLayout::_3;
	position_layout.normalized = false;
	position_layout.vertex_layout_location = 0;
	position_layout.stride = sizeof(float) * 3;
	position_layout.offset = 0;
	position_layout.relative_offset = 0;
	vertexArray.AddVertexBuffer(std::move(vertexBuffer), sizeof(float) * 3, { position_layout });

	vertexArray.Use(true);

	glViewport(0, 0, faceSize, faceSize);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (GLuint i = 0; i < 6; ++i)
	{
		glCheck(glUniformMatrix4fv(glGetUniformLocation(shaderIBL.GetProgramHandle(), "view"), 1, GL_FALSE, &views[i][0][0]));
		glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, equirectangular, 0));

		glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		glCheck(glDrawArrays(GL_TRIANGLES, 0, 36));
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	vertexArray.Use(false);
}

void GLSkybox::CalculateIrradiance()
{
	glCheck(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &irradiance));
	glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance));
	for (GLuint i = 0; i < 6; ++i)
	{
		glCheck(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, irradianceSize, irradianceSize, 0, GL_RGBA, GL_FLOAT, nullptr));
	}

	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceSize, irradianceSize);

	GLShader shaderIBL;
	shaderIBL.LoadShader({ { GLShader::VERTEX, "../Engine/shaders/glsl/Cubemap.vert" }, { GLShader::FRAGMENT, "../Engine/shaders/glsl/Irradiance.frag" } });
	shaderIBL.Use(true);

	glCheck(glUniform1i(glGetUniformLocation(shaderIBL.GetProgramHandle(), "environmentMap"), 0));
	glCheck(glUniformMatrix4fv(glGetUniformLocation(shaderIBL.GetProgramHandle(), "projection"), 1, GL_FALSE, &projection[0][0]));

	glCheck(glActiveTexture(GL_TEXTURE0));
	glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, equirectangular));

	GLVertexArray vertexArray;
	vertexArray.Initialize();

	GLVertexBuffer vertexBuffer;
	vertexBuffer.SetData(sizeof(glm::vec3) * static_cast<GLsizei>(skyboxVertices.size()), skyboxVertices.data());

	GLAttributeLayout position_layout;
	position_layout.component_type = GLAttributeLayout::Float;
	position_layout.component_dimension = GLAttributeLayout::_3;
	position_layout.normalized = false;
	position_layout.vertex_layout_location = 0;
	position_layout.stride = sizeof(float) * 3;
	position_layout.offset = 0;
	position_layout.relative_offset = 0;
	vertexArray.AddVertexBuffer(std::move(vertexBuffer), sizeof(float) * 3, { position_layout });

	vertexArray.Use(true);

	glViewport(0, 0, irradianceSize, irradianceSize);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (GLuint i = 0; i < 6; ++i)
	{
		glCheck(glUniformMatrix4fv(glGetUniformLocation(shaderIBL.GetProgramHandle(), "view"), 1, GL_FALSE, &views[i][0][0]));
		glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance, 0));

		glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		glCheck(glDrawArrays(GL_TRIANGLES, 0, 36));
	}
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	vertexArray.Use(false);
}

void GLSkybox::PrefilteredEnvironmentMap()
{
	glCheck(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &prefilter));
	glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter));
	for (GLuint i = 0; i < 6; ++i)
	{
		glCheck(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, baseSize, baseSize, 0, GL_RGBA, GL_FLOAT, nullptr));
	}

	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	glCheck(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));

	GLShader shaderIBL;
	shaderIBL.LoadShader({ { GLShader::VERTEX, "../Engine/shaders/glsl/Cubemap.vert" }, { GLShader::FRAGMENT, "../Engine/shaders/glsl/Prefilter.frag" } });
	shaderIBL.Use(true);

	glCheck(glUniform1i(glGetUniformLocation(shaderIBL.GetProgramHandle(), "environmentMap"), 0));
	glCheck(glUniformMatrix4fv(glGetUniformLocation(shaderIBL.GetProgramHandle(), "projection"), 1, GL_FALSE, &projection[0][0]));

	glCheck(glActiveTexture(GL_TEXTURE0));
	glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, equirectangular));

	GLVertexArray vertexArray;
	vertexArray.Initialize();

	GLVertexBuffer vertexBuffer;
	vertexBuffer.SetData(sizeof(glm::vec3) * static_cast<GLsizei>(skyboxVertices.size()), skyboxVertices.data());

	GLAttributeLayout position_layout;
	position_layout.component_type = GLAttributeLayout::Float;
	position_layout.component_dimension = GLAttributeLayout::_3;
	position_layout.normalized = false;
	position_layout.vertex_layout_location = 0;
	position_layout.stride = sizeof(float) * 3;
	position_layout.offset = 0;
	position_layout.relative_offset = 0;
	vertexArray.AddVertexBuffer(std::move(vertexBuffer), sizeof(float) * 3, { position_layout });

	vertexArray.Use(true);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (GLuint mip = 0; mip < mipLevels; ++mip)
	{
		uint32_t dim = baseSize >> mip;
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, dim, dim);
		glViewport(0, 0, dim, dim);

		float roughness = static_cast<float>(mip) / static_cast<float>(mipLevels - 1);
		glUniform1f(glGetUniformLocation(shaderIBL.GetProgramHandle(), "roughness"), roughness);

		for (GLuint i = 0; i < 6; ++i)
		{
			glCheck(glUniformMatrix4fv(glGetUniformLocation(shaderIBL.GetProgramHandle(), "view"), 1, GL_FALSE, &views[i][0][0]));
			glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter, mip));

			glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

			glCheck(glDrawArrays(GL_TRIANGLES, 0, 36));
		}
	}
	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	vertexArray.Use(false);
}

void GLSkybox::BRDFLUT()
{
	glCheck(glCreateTextures(GL_TEXTURE_2D, 1, &brdflut));
	glCheck(glBindTexture(GL_TEXTURE_2D, brdflut));
	glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, lutSize, lutSize, 0, GL_RG, GL_FLOAT, nullptr));

	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	glCheck(glBindFramebuffer(GL_FRAMEBUFFER, captureFBO));
	glCheck(glBindRenderbuffer(GL_RENDERBUFFER, captureRBO));
	glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, lutSize, lutSize));
	glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdflut, 0));

	GLShader shaderIBL;
	shaderIBL.LoadShader({ { GLShader::VERTEX, "../Engine/shaders/glsl/BRDF.vert" }, { GLShader::FRAGMENT, "../Engine/shaders/glsl/BRDF.frag" } });
	shaderIBL.Use(true);

	GLVertexArray vertexArray;
	vertexArray.Initialize();

	vertexArray.Use(true);

	glViewport(0, 0, lutSize, lutSize);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	glCheck(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	vertexArray.Use(false);
}
