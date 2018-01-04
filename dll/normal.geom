#version 150 compatibility

layout(triangles) in;
layout(line_strip, max_vertices = 50) out;

in VS_GS_INTERFACE{
	vec3 viewPos;
	vec3 normal;
} gs_in[];

uniform int hasTexture;
uniform int level;
uniform float radius;
uniform vec4 center;

vec3 apply_offset(vec3 p) {
	vec3 vCenter = vec3(gl_ModelViewMatrix * center);
	if (hasTexture > 0)
		return p + normalize(p - vec3(vCenter)) * (radius - distance(p, vec3(vCenter)));
	else
		return p;
}

float triangle_area(vec3 a, vec3 b, vec3 c) {
	vec3 v1 = b - a;
	vec3 v2 = c - a;
	vec3 v3 = cross(v1, v2);
	return 0.5f * length(v3);
}

void interpolate_gs_out(vec3 current_pos) {
	float areas[3];
	float sum = 0.0f;
	for (int i = 0; i < 3; ++i) {
		int next = (i + 1) % 3;
		areas[i] = triangle_area(current_pos, gs_in[i].viewPos, gs_in[next].viewPos);
		sum += areas[i];
	}

	vec3 normal = vec3(0.0f);
	for (int i = 0; i < 3; ++i) {
		normal += gs_in[i].normal * areas[(i + 1) % 3] / sum;
	}

	float normalLength = 1.0f;

	//NOTE: this is a dirty fix
	if (hasTexture > 0)
		normalLength = 0.2f;
	else
		normalLength = 10.0f;

	gl_Position = gl_ProjectionMatrix * vec4(current_pos, 1.0f);
	EmitVertex();
	gl_Position = gl_Position + gl_ProjectionMatrix * vec4(normal, 1.0f)*normalLength;
	EmitVertex();
	EndPrimitive();
}

void main() {
	float numLayer = pow(2, level);
	vec3 left_vec = (gs_in[1].viewPos - gs_in[0].viewPos) / numLayer;
	vec3 right_vec = (gs_in[2].viewPos - gs_in[0].viewPos) / numLayer;

	for (int i = 0; i < numLayer; ++i) {
		vec3 start_point = gs_in[0].viewPos + right_vec*(i + 1);
		
		interpolate_gs_out(apply_offset(start_point));		

		for (int j = 0; j < 2 + i * 2; ++j) {
			if (j % 2 == 0)
				start_point -= right_vec;
			else
				start_point += left_vec;

			interpolate_gs_out(apply_offset(start_point));
		}
	}
}