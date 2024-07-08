#include "glCheck.hpp"
#include <glew/glew.h>
#include <cassert>
#include <iostream>
#include <string>

void glCheckError(const char* file, unsigned line, const char* expression)
{
    GLenum errorCode = glGetError();

    if (errorCode == GL_NO_ERROR)
        return;

    const std::string fileString = file;
    std::string       error = "Unknown error";
    std::string       description = "No description";

    std::cerr << "An internal OpenGL call failed in " << fileString.substr(fileString.find_last_of("\\/") + 1) << "(" << line << ")."
        << "\nExpression:\n   " << expression << "\nError description:\n   ";
    int loop_limit = 0;
    while (errorCode != GL_NO_ERROR && loop_limit < 3)
    {
        ++loop_limit;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:
        {
            error = "GL_INVALID_ENUM";
            description = "An unacceptable value has been specified for an enumerated argument.";
            break;
        }

        case GL_INVALID_VALUE:
        {
            error = "GL_INVALID_VALUE";
            description = "A numeric argument is out of range.";
            break;
        }

        case GL_INVALID_OPERATION:
        {
            error = "GL_INVALID_OPERATION";
            description = "The specified operation is not allowed in the current state.";
            break;
        }

        case GL_STACK_OVERFLOW:
        {
            error = "GL_STACK_OVERFLOW";
            description = "This command would cause a stack overflow.";
            break;
        }

        case GL_STACK_UNDERFLOW:
        {
            error = "GL_STACK_UNDERFLOW";
            description = "This command would cause a stack underflow.";
            break;
        }

        case GL_OUT_OF_MEMORY:
        {
            error = "GL_OUT_OF_MEMORY";
            description = "There is not enough memory left to execute the command.";
            break;
        }

        default:
        {
            error += " " + std::to_string(errorCode);
            break;
        }
        }

        std::cerr << error << "\n   " << description << "\n\n";
        errorCode = glGetError();
    }

    assert(false);
}
