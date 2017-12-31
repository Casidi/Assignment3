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

uniform int level;
uniform float radius;
uniform vec4 center;

//TODO: do all things before projection
vec4 apply_offset(vec4 p) {
	vec3 p3 = vec3(p);
	vec3 center3 = vec3(center);

	vec4 pCenter = gl_ProjectionMatrix * gl_ModelViewMatrix * center;
	//return p;
	return p + normalize(p - pCenter)*(radius - distance(p, pCenter));
}

float triangle_area(vec4 a, vec4 b, vec4 c) {
	vec3 v1 = vec3(b - a);
	vec3 v2 = vec3(c - a);
	vec3 v3 = cross(v1, v2);
	return 0.5f * length(v3);
}

void interpolate_gs_out(vec4 current_pos) {
	float areas[3];
	float sum = 0.0f;
	for(int i = 0; i < 3; ++i) {
		int next = (i + 1) % 3;
		areas[i] = triangle_area(current_pos, gl_in[i].gl_Position, gl_in[next].gl_Position);
		sum += areas[i];
	}
	
	gs_out.viewPos = vec3(0.0f);
	gs_out.normal = vec3(0.0f);
	gs_out.tex_coord = vec2(0.0f);
	for (int i = 0; i < 3; ++i) {
		gs_out.viewPos += gs_in[i].viewPos * areas[(i + 1) % 3] / sum;
		gs_out.normal += gs_in[i].normal * areas[(i + 1) % 3] / sum;
		gs_out.tex_coord += gs_in[i].tex_coord * areas[(i + 1) % 3] / sum;
	}
}

void main() {
	gs_out.tex_coord = vec2(0.0f);
	vec4 left_vec = (gl_in[1].gl_Position - gl_in[0].gl_Position)/pow(2, level);
	vec4 right_vec = (gl_in[2].gl_Position - gl_in[0].gl_Position)/pow(2, level);
	
	for(int i = 0; i < pow(2, level); ++i) {
		vec4 start_point = gl_in[0].gl_Position + right_vec*(i+1);
		gl_Position = apply_offset(start_point);
		interpolate_gs_out(apply_offset(start_point));
		EmitVertex();
		
		for(int j = 0; j < 2 + i*2; ++j) {
			if(j%2 == 0)
				start_point -= right_vec;
			else
				start_point += left_vec;
				
			gl_Position = apply_offset(start_point);
			interpolate_gs_out(apply_offset(start_point));
			EmitVertex();
		}
		EndPrimitive();
	}
}