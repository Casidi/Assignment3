#pragma once

#include <string>
#include <fstream>
#include <gl\GL.h>
#include "Utilityies.h"

using namespace std;

class Camera {
public:
	Camera(string filepath) {
		ifstream file(filepath.c_str());
		char buffer[16];
		file >> buffer >> pos[0] >> pos[1] >> pos[2];
		file >> buffer >> target[0] >> target[1] >> target[2];
		file >> buffer >> up[0] >> up[1] >> up[2];
		file >> buffer >> fovy;
		file >> buffer >> near;
		file >> buffer >> far;
		file >> buffer >> viewport[0] >> viewport[1] >> viewport[2] >> viewport[3];
		file.close();
	}

	void setProjection() {
		glPushAttrib(GL_TRANSFORM_BIT);
		glPushMatrix();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fovy, (double)viewport[2] / (double)viewport[3], near, far);	

		glPopAttrib();
		glPopMatrix();
	}

	void reshape(int w, int h) {
		viewport[2] = w;
		viewport[3] = h;
		glViewport(0, 0, w, h);
		setProjection();
	}

	void setTarget(float x, float y, float z) {
		target[0] = x;
		target[1] = y;
		target[2] = z;
	}

	void setViewMatrix(int jitterPass) {
		float viewVec[3];
		for (int i = 0; i < 3; ++i)
			viewVec[i] = pos[i] - target[i];

		float right[3];
		cross(up, viewVec, right);
		
		normalize(up);
		normalize(right);

		float jitterRadius = 0.2f;
		float offset[3];
		float jitterAngle = 3.14159265f / 4.0f * (float)jitterPass;
		for (int i = 0; i < 3; ++i)
			offset[i] = (right[i] * cos(jitterAngle) + up[i] * sin(jitterAngle))*jitterRadius;

		gluLookAt(pos[0] + offset[0], pos[1] + offset[1], pos[2] + offset[2],
			target[0], target[1], target[2],
			up[0], up[1], up[2]);
	}

	void zoomIn(int step) {
		if (fovy < 180.0f && fovy > 5.0f)
			fovy -= step;
		else if (fovy <= 6.0f && step < 0.0f)
			fovy -= step;
		else if (fovy >= 179.0f && step > 0.0f)
			fovy -= step;
	}

	float pos[3], target[3], up[3];
	float fovy, near, far;
	int viewport[4];
};
