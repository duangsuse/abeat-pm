#pragma once

namespace abeat::shader {

const char *VERT_GRAVITY = R"(
#version 330

layout(location = 0) in float y_in;
layout(location = 1) in float t_in;
out float y_out;
out float t_out;

uniform samplerBuffer fft_buf;
uniform float u_dt;

float gt(float b, float a) { return max(sign(a - b), 0.0); }

float acc(float t) { return -8.0 * t; }
void main() {
	float y = texelFetch(fft_buf, gl_VertexID).x;
	float t = max(t_in, 0.0); // t used

	// RK4 integration
	float k1 = acc(t);
	float k2 = acc(t + u_dt * 0.5);
	float k4 = acc(t + u_dt);
	float dy_dt = 1./6. * (k1 + 4. * k2 + k4);

	float yg = y_in + dy_dt * u_dt;
	//t = t + u_dt;

	// set output variables
	float gt_y = gt(yg, y);
	y_out = max(yg, y);
	t_out = mix(t+u_dt, 0., gt_y);
}
)";

const char *VERT_BAR = R"(
#version 330

layout(location = 0) in float y_in;

uniform int u_count;

void main() {
	float x = mix(-1., 1., (float(gl_VertexID) + 0.5) / u_count);
	float y = mix(-1., 1., clamp(y_in, 0, 1));
	gl_Position = vec4(x, y, 0, 1);
}
)";

const char *FRAG_BAR = R"(
#version 330

out vec4 color_out;

void main() {
	float alpha = 1;
	color_out = vec4(0.01171875*alpha, 0.66015625*alpha, 0.953125*alpha, alpha);
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
