#include "render/shader.h"

#include <glad/glad.h>

#include <cstdio>

namespace spacewar::render {
namespace {

// Compiles one stage; returns 0 (after logging) on failure so the caller aborts
// construction instead of proceeding with a broken program.
unsigned int CompileShader(unsigned int type, const char* src) {
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);
  int ok;
  glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[512];
    glGetShaderInfoLog(id, sizeof(log), nullptr, log);
    fprintf(stderr, "shader compile failed: %s\n", log);
    glDeleteShader(id);
    return 0;
  }
  return id;
}

}  // namespace

Shader::Shader(const char* vertex_src, const char* fragment_src) {
  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertex_src);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragment_src);
  if (vs == 0 || fs == 0) {
    glDeleteShader(vs);
    glDeleteShader(fs);
    return;
  }

  program_ = glCreateProgram();
  glAttachShader(program_, vs);
  glAttachShader(program_, fs);
  glLinkProgram(program_);
  glDeleteShader(vs);
  glDeleteShader(fs);

  int ok;
  glGetProgramiv(program_, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[512];
    glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
    fprintf(stderr, "shader linking failed: %s\n", log);
    glDeleteProgram(program_);
    program_ = 0;
  }
}

Shader::~Shader() { glDeleteProgram(program_); }

void Shader::SetMat3(const char* name, const Mat3& m) const {
  int loc = glGetUniformLocation(program_, name);
  glUniformMatrix3fv(loc, 1, GL_FALSE, &m.cols[0].x);
}

void Shader::SetColor(const char* name, const Color& c) const {
  int loc = glGetUniformLocation(program_, name);
  glUniform4f(loc, c.r, c.g, c.b, c.a);
}

void Shader::SetInt(const char* name, int value) const {
  int loc = glGetUniformLocation(program_, name);
  glUniform1i(loc, value);
}

void Shader::Use() const { glUseProgram(program_); }

}  // namespace spacewar::render
