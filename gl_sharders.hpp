#pragma once

namespace abeat::shader {

const char *VERT_GRAVITY = R"(
#version 330

layout(location = 0) in float i_y;
layout(location = 1) in float i_t;

out float o_y;
out float o_t;

uniform samplerBuffer fft_tbo;
uniform float u_dt;

float gt(float x, float y) {
	return max(sign(x - y), 0.0);
}
float acc(float t) {
	return -8.0 * t;
}
void main() {
	float y = texelFetch(fft_tbo, gl_VertexID).x;
	float time = max(i_t, 0.0);

	// RK4 integration
	float k1 = acc(time);
	float k2 = acc(time + u_dt * 0.5);
	float k4 = acc(time + u_dt);
	float dy_dt = 1./6. * (k1 + 4. * k2 + k4);

	float yg = i_y + dy_dt * u_dt;
	time = time + u_dt;

	// set output variables
	float gt_y = gt(y, yg);
	o_y = max(yg, y);
	o_t = mix(time, 0., gt_y);
}
)";

const char *VERT_BAR = R"(
#version 330

layout(location = 0) in float i_y;

uniform int u_count;

void main() {
	float x = mix(-1., 1., (float(gl_VertexID) + 0.5) / u_count);
	float y = mix(-1., 1., clamp(i_y, 0, 1));
	gl_Position = vec4(x, y, 0, 1);
}
)";

const char *FRAG_BAR = R"(
#version 330

out vec4 o_color;

void main() {
	float alpha = 1;
	o_color = vec4(0.01171875 * alpha, 0.66015625 * alpha, 0.953125 * alpha, alpha);
}
)";

const char *GEOM_BAR = R"(
#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform int u_count;
uniform float u_half;

void main() {
	float l = gl_in[0].gl_Position.x - u_half;
	float r = gl_in[0].gl_Position.x + u_half;
	float y = gl_in[0].gl_Position.y;

	gl_Position = vec4(l, -1, 0, 1); EmitVertex();
	gl_Position = vec4(r, -1, 0, 1); EmitVertex();
	gl_Position = vec4(l, y, 0, 1); EmitVertex();
	gl_Position = vec4(r, y, 0, 1); EmitVertex();
	EndPrimitive();
}
)";

} // namespace abeat::shader
