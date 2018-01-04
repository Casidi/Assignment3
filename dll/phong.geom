#version 150 compatibility

layout(triangles) in;
layout(triangle_strip, max_vertices=50) out;

in VS_GS_INTERFACE {
	vec3 viewPos;
	vec3 normal;
	vec2 tex_coord;
} gs_in[];

out GS_FS_INTERFACE {
	vec3 viewPos;
	vec3 normal;
	vec2 tex_coord;
} gs_out;

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

float triangle_area_2(vec3 a, vec3 b, vec3 c) {
	vec3 v1 = b - a;
	vec3 v2 = c - a;
	vec3 v3 = cross(v1, v2);
	return length(v3);
}

void interpolate_gs_out(vec3 current_pos) {
	float areas[3];
	float sum = 0.0f;
	for(int i = 0; i < 3; ++i) {
		int next = (i + 1) % 3;
		areas[i] = triangle_area_2(current_pos, gs_in[i].viewPos, gs_in[next].viewPos);
		sum += areas[i];
	}
	
	gs_out.viewPos = vec3(0.0f);
	gs_out.normal = vec3(0.0f);
	gs_out.tex_coord = vec2(0.0f);
	for (int i = 0; i < 3; ++i) {
		float weight = areas[(i + 1) % 3] / sum;
		gs_out.viewPos += gs_in[i].viewPos * weight;
		gs_out.normal += gs_in[i].normal * weight;
		gs_out.tex_coord += gs_in[i].tex_coord * weight;
	}
}

void main() {
	float numLayer = pow(2, level);
	vec3 left_vec = (gs_in[1].viewPos - gs_in[0].viewPos)/numLayer;
	vec3 right_vec = (gs_in[2].viewPos - gs_in[0].viewPos)/numLayer;
	
	for(int i = 0; i < numLayer; ++i) {
		vec3 start_point = gs_in[0].viewPos + right_vec*(i+1);
		gl_Position = gl_ProjectionMatrix * vec4(apply_offset(start_point), 1.0f);
		interpolate_gs_out(apply_offset(start_point));
		EmitVertex();
		
		for(int j = 0; j < 2 + i*2; ++j) {
			if(j%2 == 0)
				start_point -= right_vec;
			else
				start_point += left_vec;
				
			gl_Position = gl_ProjectionMatrix * vec4(apply_offset(start_point), 1.0f);
			interpolate_gs_out(apply_offset(start_point));
			EmitVertex();
		}
		EndPrimitive();
	}
}