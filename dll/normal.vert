#version 150 compatibility

out VS_GS_INTERFACE{
	vec3 viewPos;
	vec3 normal;
} vs_out;

void main() {
	vs_out.viewPos = vec3(gl_ModelViewMatrix * gl_Vertex);
	vs_out.normal = normalize(gl_NormalMatrix * gl_Normal);
}