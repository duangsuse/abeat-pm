#version 330
// vert
layout(location = 0) in float y_in;

uniform int u_count;

void main() {
	float x = mix(-1., 1., (float(gl_VertexID) + 0.5) / u_count);
	float y = mix(-1., 1., clamp(y_in, 0, 1));
	gl_Position = vec4(x, y, 0, 1);
}

// frag
out vec4 color_out;

void main() {
	float alpha = 0.5;
	color_out = vec4(0.01171875*alpha, 0.66015625*alpha, 0.953125*alpha, alpha);
}

// geom
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

