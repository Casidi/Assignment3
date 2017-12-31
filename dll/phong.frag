#version 150 compatibility

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
		vec3 lightDir = normalize(gl_LightSource[i].position.xyz - fs_in.viewPos);
		vec3 reflectionDir = normalize(reflect(-lightDir, normDir));
		
		vec4 ambient = gl_LightSource[i].ambient * gl_FrontMaterial.ambient;
		vec4 diffuse = gl_LightSource[i].diffuse * gl_FrontMaterial.diffuse * max(dot(normDir, lightDir), 0.0f);
		vec4 specular = gl_LightSource[i].specular * gl_FrontMaterial.specular * pow(max(dot(reflectionDir, viewDir), 0.0f), gl_FrontMaterial.shininess);
		
		sum += ambient + diffuse + specular;
	}
	
	//gl_FragColor = texture(color_texture, gl_TexCoord[0].xy);
	//gl_FragColor = texture(color_texture, fs_in.tex_coord)*sum;
	//gl_FragColor = vec4(fs_in.tex_coord.x, 0, 0, 0);
	gl_FragColor = sum;
}