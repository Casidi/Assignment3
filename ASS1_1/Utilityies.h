#pragma once

void normalize(float* v) {
	float len = sqrtf(v[0] * v[0]
		+ v[1] * v[1]
		+ v[2] * v[2]);
	for (int i = 0; i < 3; ++i)
		v[i] /= len;
}

void cross(float* a, float* b, float* result) {
	result[0] = a[1] * b[2] - a[2] * b[1];
	result[1] = a[2] * b[0] - a[0] * b[2];
	result[2] = a[0] * b[1] - a[1] * b[0];
}

float distance(float* a, float* b) {
	float temp = 0.0f;
	for (int i = 0; i < 3; ++i)
		temp += (a[i] - b[i])*(a[i] - b[i]);
	return sqrtf(temp);
}
