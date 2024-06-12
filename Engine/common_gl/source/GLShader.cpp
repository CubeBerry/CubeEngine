//Author: JEYOON YU
//Project: CubeEngine
//File: GLShader.hpp
#include "GLShader.hpp"
#include <fstream>

GLShader::~GLShader()
{
    if (programHandle > 0)
    {
        glDeleteProgram(programHandle);
        programHandle = 0;
    }
}

void GLShader::Use(bool bind) const noexcept
{
    glUseProgram(bind ? programHandle : 0);
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
                //error_log = "Cannot find " + path.string() + "\n";
                //return false;
            }
            std::ifstream ifs(path.second, std::ios::in);
            if (!ifs)
            {
                //error_log = "Cannot open " + path.string() + "\n";
                //return false;
            }
            std::string glsl_text;
            glsl_text.reserve(std::filesystem::file_size(path.second));
            std::copy((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>(), std::back_insert_iterator(glsl_text));

            if (shaders[count] <= 0)
            {
                shaders[count] = glCreateShader(path.first);
                if (shaders[count] == 0)
                {
                    //error_log = "Unable to create shader\n";
                }
            }
            const GLchar* source[]{ glsl_text.data() };
            glShaderSource(shaders[count], 1, source, nullptr);
            glCompileShader(shaders[count]);

            GLint isCompiled{ 0 };
            glGetShaderiv(shaders[count], GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                //GLint log_length = 0;
                //glCheck(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length));
                //error_log.resize(static_cast<std::string::size_type>(log_length) + 1);
                //glCheck(glGetShaderInfoLog(shader, log_length, nullptr, error_log.data()));
                glDeleteShader(shaders[count]);
                //shader = 0;
            }

            count++;
        }

        //Link Program
        programHandle = glCreateProgram();
        if (programHandle == 0)
        {
            //throw std::runtime_error("Unable to create program\n");
        }

        for (const auto shader : shaders)
        {
            glAttachShader(programHandle, shader);
        }

        glLinkProgram(programHandle);
        for (auto& shader : shaders)
        {
            glDeleteShader(shader);
        }
        GLint isLinked{ 0 };
        glGetProgramiv(programHandle, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            //GLint log_length = 0;
            //glCheck(glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_length));
            //std::string error;
            //error.resize(static_cast<unsigned>(log_length) + 1);
            //glCheck(glGetProgramInfoLog(program_handle, log_length, nullptr, error.data()));
            //throw std::runtime_error(error);
        }
    }
    catch (std::exception& e)
    {
        e;
    }
}
