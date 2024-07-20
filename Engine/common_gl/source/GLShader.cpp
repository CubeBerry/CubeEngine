//Author: JEYOON YU
//Second Author: DOYEONG LEE
//Project: CubeEngine
//File: GLShader.hpp
#include "GLShader.hpp"
#include <fstream>
#include <iostream>

#include "glCheck.hpp"

GLShader::~GLShader()
{
    if (programHandle > 0)
    {
        glCheck(glDeleteProgram(programHandle));
        programHandle = 0;
    }
}

void GLShader::Use(bool bind) const noexcept
{
    //glCheck(glUseProgram(bind ? programHandle : 0));
    if (bind == true)
    {
        glCheck(glUseProgram(programHandle));
    }
    else if (bind == false)
    {
        glCheck(glUseProgram(0));
    }
}

void GLShader::LoadShader(const std::initializer_list<std::pair<GLShader::Type, std::filesystem::path>>& paths)
{
    GLuint count{ 0 };
    std::vector<GLuint> shaders(paths.size(), 0);

    try
    {
        for (auto& path : paths)
        {
            if (!std::filesystem::exists(path.second))
            {
                throw std::runtime_error{ "Shader Path Does Not Exist" };
            }
            std::ifstream ifs(path.second, std::ios::in);
            if (!ifs)
            {
                throw std::runtime_error{ "Unable To Create ifstream" };
            }
            std::string glsl_text;
            glsl_text.reserve(std::filesystem::file_size(path.second));
            std::copy((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>(), std::back_insert_iterator(glsl_text));

            if (shaders[count] <= 0)
            {
                shaders[count] = glCheck(glCreateShader(path.first));
                if (shaders[count] == 0)
                {
                    throw std::runtime_error{ "Shader Creation Failed" };
                }
            }
            const GLchar* source[]{ glsl_text.data() };
            glCheck(glShaderSource(shaders[count], 1, source, nullptr));
            glCheck(glCompileShader(shaders[count]));

            GLint isCompiled{ 0 };
            glCheck(glGetShaderiv(shaders[count], GL_COMPILE_STATUS, &isCompiled));
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glCheck(glGetShaderiv(shaders[count], GL_INFO_LOG_LENGTH, &maxLength));
                std::vector<GLchar> infoLog(maxLength);

                glCheck(glGetShaderInfoLog(shaders[count], maxLength, nullptr, infoLog.data()));
                std::cerr << "COMPILE FAILED\n" << infoLog.data() << std::endl;

                glCheck(glDeleteShader(shaders[count]));

                throw std::runtime_error{ "Shader Compile Failed" };
            }

            count++;
        }

        //Link Program
        programHandle = glCheck(glCreateProgram());
        if (programHandle == 0)
        {
            throw std::runtime_error{ "Shader Program Create Failed" };
        }

        for (const auto shader : shaders)
        {
            glCheck(glAttachShader(programHandle, shader));
        }

        glCheck(glLinkProgram(programHandle));
        for (auto& shader : shaders)
        {
            glCheck(glDeleteShader(shader));
        }

        GLint isLinked{ 0 };
        glCheck(glGetProgramiv(programHandle, GL_LINK_STATUS, &isLinked));
        if (isLinked == GL_FALSE)
        {
            char infoLog[512];
            glCheck(glGetProgramInfoLog(programHandle, 512, NULL, infoLog));
            std::cerr << "LINK FAILED\n" << infoLog << std::endl;

            throw std::runtime_error{ "Shader Link Failed" };
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        GLShader::~GLShader();
        std::exit(EXIT_FAILURE);
    }
}