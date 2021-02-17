#version 330
// vert
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

