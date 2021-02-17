#ifdef PKGED
#  include "pkged/gl_my.hpp"
#else
#  include "gl_my.hpp"
#endif
#include "gl_sharders.hpp"

namespace abeat {

class Render {
public:
  explicit Render(size_t size);
  ~Render();
  void render(float dt);

  float *data;
  const size_t size;
private:
  mgl::Texture texture;
  mgl::Buffer b_fft, b_tf[2];
  mgl::VAO v_bar[2], v_bar_g[2];
  mgl::Program p_bar, p_bar_g;
  int i_flipy;
};
#ifndef PKGED
Render::~Render() { delete[] data; }
void Render::render(float dt) {
  // glClearColor(0, 0, 0, 0);
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  b_fft.bind(GL_TEXTURE_BUFFER);
  glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(float) * size, data);
  mgl::Buffer::unbind(GL_TEXTURE_BUFFER);

  p_bar_g.use();
  glUniform1f(p_bar_g.locate("u_dt"), dt);

  v_bar_g[i_flipy].bind();
  b_tf[i_flipy].bind(GL_TRANSFORM_FEEDBACK_BUFFER);

  glActiveTexture(GL_TEXTURE0);
  texture.bind(GL_TEXTURE_BUFFER);

  glBeginTransformFeedback(GL_POINTS);
  glEnable(GL_RASTERIZER_DISCARD);
  glDrawArrays(GL_POINTS, 0, size); // here
  glDisable(GL_RASTERIZER_DISCARD);
  glEndTransformFeedback();

  mgl::Texture::unbind(GL_TEXTURE_BUFFER);
  mgl::Buffer::unbind(GL_TRANSFORM_FEEDBACK_BUFFER);

  p_bar.use();
  v_bar[i_flipy].bind();
  glDrawArrays(GL_POINTS, 0, size); // here
  i_flipy ^= 1;

  mgl::VAO::unbind();
}

Render::Render(size_t size): size(size), i_flipy(0) {
  data = new float[size];
  std::fill(data, data+size, 0);

  {
    std::ifstream barGlsl("res/bar.shader");
    if(barGlsl.is_open()) p_bar.loadShaders(barGlsl);
    else {
      mgl::Shader
        vertex_shader(shader::VERT_BAR, GL_VERTEX_SHADER),
        geometry_shader(shader::GEOM_BAR, GL_GEOMETRY_SHADER),
        fragment_shader(shader::FRAG_BAR, GL_FRAGMENT_SHADER);
      p_bar.link(vertex_shader, geometry_shader, fragment_shader);
    }
    p_bar.use();
    glUniform1i(p_bar.locate("u_count"), size);
    glUniform1f(p_bar.locate("u_half"), 1.0f / size/* *0.7 */);
  }
  {
    const char *varyings[] = {"y_out", "t_out"};
    glTransformFeedbackVaryings(p_bar_g.id, 2, varyings, GL_INTERLEAVED_ATTRIBS);
    std::ifstream bargGlsl("res/barg.shader");
    if(bargGlsl.is_open()) p_bar_g.loadShaders(bargGlsl);
    else {
      mgl::Shader shader(shader::VERT_GRAVITY, GL_VERTEX_SHADER);
      p_bar_g.link(shader);
    }
    p_bar_g.use();
    
    glUniform1i(p_bar_g.locate("fft_buf"), 0/*b_fft*/);
  }
  for (int i=0; i<2; i++) {
    b_tf[i].bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size * 2, 0, GL_STREAM_DRAW);

    v_bar[i].bind();
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
    glEnableVertexAttribArray(0);
    v_bar[i].unbind();

    v_bar_g[!i].bind(); // NOTE: !(int)x used
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)sizeof(float));
    glEnableVertexAttribArray(1);
    v_bar_g[!i].unbind();

    mgl::Buffer::unbind();
  }
  b_fft.bind(GL_TEXTURE_BUFFER);
  glBufferData(GL_TEXTURE_BUFFER, sizeof(float) * size, 0, GL_STREAM_DRAW);
  mgl::Buffer::unbind(GL_TEXTURE_BUFFER);

  texture.bind(GL_TEXTURE_BUFFER);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, b_fft.id);
  mgl::Texture::unbind(GL_TEXTURE_BUFFER);
}
#endif
} // namespace abeat
