#pragma once

void glCheckError(const char* file, unsigned int line, const char* expression);

// Note that this macro does expression, semicolon, and then calls glCheckError,
// so you shouldn't use this macro under an if statement without { } and
// you shouldn't use it in a return statement.
// Use it like
//      glCheck(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val));
// or
//      glCheck(GLubyte const* param_str = glGetString(GL_VENDOR));
#if defined(_DEBUG) || defined(DEBUG)
#define glCheck(expr)                                                                                                                                                              \
    expr;                                                                                                                                                                          \
    glCheckError(__FILE__, __LINE__, #expr)
#else
#define glCheck(expr) expr
#endif
