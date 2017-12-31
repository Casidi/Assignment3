#pragma once

//An object can be seen as an instance of a model
class Object {
	struct vec3f {
		float x, y, z;
	};

public:
	int modelIndex;
	int textureIndex;
	vec3f scale, rotation, translation;
	float angle;
};