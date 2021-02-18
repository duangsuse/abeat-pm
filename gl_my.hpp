#include <stdexcept>
#include <string>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define OBJ(name) \
  name(name &&t): id(t.id) { t.id = 0; } \
  name(const name &) = delete; \
  name &operator=(name &&) noexcept = default;

namespace mgl {

struct VAO { GLuint id;
  VAO() { glGenVertexArrays(1, &id); }
  ~VAO() { glDeleteVertexArrays(1, &id); }
  OBJ(VAO)

  inline void bind() const { glBindVertexArray(id); }
  static inline void unbind() { glBindVertexArray(0); }
};

struct Buffer { GLuint id;
  Buffer() { glGenBuffers(1, &id); }
  ~Buffer() { glDeleteBuffers(1, &id); }
  OBJ(Buffer)

  inline void bind(int type = GL_ARRAY_BUFFER) const {
    if (type == GL_TRANSFORM_FEEDBACK_BUFFER) glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, id);
    else glBindBuffer(type, id);
  }
  static inline void unbind(int type = GL_ARRAY_BUFFER) { glBindBuffer(type, 0); }
};

struct Texture { GLuint id;
  Texture() { glGenTextures(1, &id); }
  ~Texture() { glDeleteTextures(1, &id); }
  OBJ(Texture)

  inline void bind(int type) const { glBindTexture(type, id); }
  static inline void unbind(int type) { glBindTexture(type, 0); }
};

struct Shader { GLuint id;
  Shader(const char *code, GLuint type); //<v key types
  ~Shader() { if (id) glDeleteShader(id); }
  OBJ(Shader)
};

struct Program { GLuint id;
  Program() { if (!(id = glCreateProgram())) throw std::runtime_error("Failed to create shader program"); }
  ~Program() { if (id) glDeleteProgram(id); }
  OBJ(Program)

  inline void attach(const Shader &shader) { glAttachShader(id, shader.id); }
  template<typename...Args>
  inline void attach(const Shader &sh, Args &... shs) { // recursive expand
      attach(sh);
      attach(shs...);
  };
  inline void detach(const Shader &shader) { glDetachShader(id, shader.id); }
  template<typename...Args>
  inline void detach(const Shader &sh, Args &... shs) {
      detach(sh);
      detach(shs...);
  };

  template<typename...Args>
  void link(Args &... shs) {
      attach(shs...);
      glLinkProgram(id);
      detach(shs...);
      check_link();
  }

  void check_link() {
    GLint link_status;
    glGetProgramiv(id, GL_LINK_STATUS, &link_status);

    if (link_status == GL_FALSE) {
        GLint length;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
        std::string log;
        log.resize(length);
        glGetProgramInfoLog(id, length, &length, log.data());
        throw std::invalid_argument(log);
    }
  }

  inline GLuint locate(const char *name) const { return glGetUniformLocation(id, name); }
  inline void use() const { glUseProgram(id); }
#undef OBJ
  void loadShaders(std::ifstream& file) {
    std::string headDef;
    std::string* codes[3] = {nullptr}; // vert,geom,frag
    std::string line;
    auto addTo = [&line](std::string& s) { s.append(line); s.push_back('\n'); };
    while (!file.eof()) {
      std::getline(file, line);
      if (line.find("// ") == std::string::npos) { addTo(headDef); }
      else {
        auto name = line.substr(3);
        int i = (name=="vert")? 0 :(name=="geom")? 1 :(name=="frag")? 2 :-1;
        if (i==-1) { addTo(headDef); continue; }
        if (codes[i] != nullptr) { line+=" duplicate"; throw std::invalid_argument(line); }
        else {
          codes[i] = new std::string(headDef);
          while (!file.eof()) {
            std::getline(file, line); if (*line.begin() == '\x03') break;
            addTo(*codes[i]);
          }
        }
      }
    }
    int types[] = {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER};
    Shader* shads[3] = {};
    for (int i=0;i<3;i++) {
      auto code = codes[i];
      if (code!=nullptr) { shads[i]=new Shader(code->c_str(), types[i]); attach(*shads[i]); } //mem
    }
    glLinkProgram(id);
    for (int i=0;i<3;i++) { if (codes[i]==nullptr)continue; detach(*shads[i]); delete shads[i]; delete codes[i]; }
    check_link();
  }
};
#ifndef PKGED
Shader::Shader(const char *code, GLuint type) {
  if (!(id = glCreateShader(type))) throw std::runtime_error("Failed to create shader");
  glShaderSource(id, 1, &code, nullptr);
  glCompileShader(id);
  GLint status;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
      GLint length;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
      std::string str;
      str.resize(length);
      glGetShaderInfoLog(id, length, &length, str.data());
      throw std::invalid_argument(str);
  }
}
#endif
} // namespace mgl
