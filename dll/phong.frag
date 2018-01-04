#version 150 compatibility

//dirty
uniform int hasTexture;

uniform int lightNumber;
uniform sampler2D color_texture;

in GS_FS_INTERFACE {
	vec3 viewPos;
	vec3 normal;
	vec2 tex_coord;
} fs_in;

void main() {
	vec3 normDir = normalize(fs_in.normal);
	vec3 viewDir = normalize(-fs_in.viewPos);
	vec4 sum = vec4(0.0f);
	for(int i = 0; i < lightNumber; ++i) {
		vec3 lightDir = normalize(vec3(gl_LightSource[i].position) - fs_in.viewPos);
		vec3 reflectionDir = normalize(reflect(-lightDir, normDir));
		
		vec4 ambient = gl_LightSource[i].ambient * gl_FrontMaterial.ambient;
		vec4 diffuse = gl_LightSource[i].diffuse * gl_FrontMaterial.diffuse * max(dot(normDir, lightDir), 0.0f);
		vec4 specular = gl_LightSource[i].specular * gl_FrontMaterial.specular * pow(max(dot(reflectionDir, viewDir), 0.0f), gl_FrontMaterial.shininess);
		
		sum += ambient + diffuse + specular;
	}
	
	//NOTD: This is a dirty fix
	if(hasTexture > 0)
		gl_FragColor = texture(color_texture, fs_in.tex_coord)*sum;
	else
		gl_FragColor = sum;
}