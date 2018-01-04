#pragma once
#include <vector>
#include <fstream>
#include <string>

#include "Texture.h"
#include "Object.h"
#include "Shader.h"
#include "GL/glew.h"

using namespace std;

class Light {
public:
	float pos[4];
	float ambient[4];
	float diffuse[4];
	float specular[4];

	Light() {
		pos[3] = ambient[3] = diffuse[3] = specular[3] = 1.0f;
	}
};

class LightCollection {
public:
	vector<Light> lights;
	float env_ambient[4];

	LightCollection(string filepath) {
		ifstream file(filepath);
		string buffer;
		int count = 0;
		while (!file.eof()) {
			file >> buffer;
			if (buffer == "")
				break;
			else if (buffer == "light") {
				Light light;

				for (int i = 0; i < 3; ++i)
					file >> light.pos[i];
				for (int i = 0; i < 3; ++i)
					file >> light.ambient[i];
				for (int i = 0; i < 3; ++i)
					file >> light.diffuse[i];
				for (int i = 0; i < 3; ++i)
					file >> light.specular[i];

				printf("Light %d loaded\n", count);
				printf("Position=(%f,%f,%f)\n", light.pos[0], light.pos[1], light.pos[2]);

				lights.push_back(light);
				++count;
			}
			else if (buffer == "ambient") {
				env_ambient[3] = 1.0f;
				file >> env_ambient[0] >> env_ambient[1] >> env_ambient[2];
			}
		}
		file.close();
	}
};

class Scene {
public:
	Scene(string filepath, string lightFileName, string cameraFileName) {
		ifstream file(filepath.c_str());
		string buffer;
		int currentTextureIndex = -1;
		while (!file.eof()) {
			file >> buffer;
			if (buffer[0] == 0)
				break;

			if (buffer == "model") {
				Object* newObject = new Object();
				file >> buffer; //model file name

				vector<string>::iterator iter = find(modelNames.begin(), modelNames.end(), buffer);
				if (iter == modelNames.end()) {
					//model hasn't been loaded
					mesh *object = new mesh(buffer.c_str());
					models.push_back(object);
					modelNames.push_back(buffer);

					newObject->modelIndex = modelNames.size() - 1;
				}
				else {
					newObject->modelIndex = iter - modelNames.begin();
				}

				file >> newObject->scale.x >> newObject->scale.y >> newObject->scale.z;
				file >> newObject->angle >> newObject->rotation.x >> newObject->rotation.y >> newObject->rotation.z;
				file >> newObject->translation.x >> newObject->translation.y >> newObject->translation.z;

				newObject->textureIndex = currentTextureIndex;

				objects.push_back(newObject);
			}
			else {
				if (buffer == "no-texture") {
					currentTextureIndex = -1;
				}
				else if (buffer == "single-texture") {
					currentTextureIndex = textures.size();

					file >> buffer;
					SingleTexture* t = new SingleTexture(buffer);
					textures.push_back(t);
				}
				else if (buffer == "multi-texture") {
					currentTextureIndex = textures.size();

					string t1, t2;
					file >> t1 >> t2;
					MultiTexture* t = new MultiTexture(t1, t2);
					textures.push_back(t);
				}
				else if (buffer == "cube-map") {
					currentTextureIndex = textures.size();

					vector<string> pathes;
					for (int i = 0; i < 6; ++i) {
						file >> buffer;
						pathes.push_back(buffer);
					}

					CubeMap* t = new CubeMap(pathes);
					textures.push_back(t);
				}
				else {
					currentTextureIndex = -1;
				}
			}
		}
		file.close();

		lightCollection = new LightCollection(lightFileName);
		camera = new Camera(cameraFileName);
		shader = new Shader("phong.vert", "phong.frag", "phong.geom");
		shaderNormal = new Shader("normal.vert", "normal.frag", "normal.geom");

		float centroid[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

		int level = 2;
		float radius = 1.0f;
		shader->use();
		shader->setUniform("lightNumber", (int)lightCollection->lights.size());
		shader->setUniform("level", level);
		shader->setUniform("radius", radius);
		shader->setUniform("center", centroid);
		shader->setUniform("color_texture", 0);
		shader->setUniform("hasTexture", (int)textures.size());

		shaderNormal->use();
		shaderNormal->setUniform("center", centroid);
		shaderNormal->setUniform("radius", radius);
		shaderNormal->setUniform("level", level);
		shaderNormal->setUniform("hasTexture", (int)textures.size());
	}

	~Scene() {
		delete lightCollection;
		delete camera;
		delete shader;
		delete shaderNormal;
		for (int i = 0; i < models.size(); ++i)
			delete models[i];
		for (int i = 0; i < objects.size(); ++i)
			delete objects[i];
		for (int i = 0; i < textures.size(); ++i)
			delete textures[i];
	}
	
	
	void renderObject(Object *obj, int reflectionPass) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		setLighting();

		//reflection
		glTranslatef(-40.0f * 2.0f * reflectionPass, 0, 0);
		if (abs(reflectionPass) % 2 == 0) {
			glScalef(1, 1, 1);
			glFrontFace(GL_CCW);
		}
		else {
			glScalef(-1, 1, 1);
			glFrontFace(GL_CW);
		}

		//model transformation
		glTranslatef(obj->translation.x, obj->translation.y, obj->translation.z);
		glRotatef(obj->angle, obj->rotation.x, obj->rotation.y, obj->rotation.z);
		glScalef(obj->scale.x, obj->scale.y, obj->scale.z);		

		int lastMaterial = -1;
		auto model = models[obj->modelIndex];
		for (size_t j = 0; j < model->fTotal; ++j)
		{
			auto face = model->faceList[j];
			if (lastMaterial != face.m)
			{
				lastMaterial = (int)face.m;
				glMaterialfv(GL_FRONT, GL_AMBIENT, model->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, model->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR, model->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &model->mList[lastMaterial].Ns);
			}

			if (obj->textureIndex != -1)
				textures[obj->textureIndex]->use();

			glBegin(GL_TRIANGLES);
			for (size_t k = 0; k<3; ++k)
			{
				if (obj->textureIndex != -1) {
					float* point = model->tList[face[k].t].ptr;
					if (textures[obj->textureIndex]->type == 1) {
						glTexCoord2f(point[0], point[1]);
					}
					else if (textures[obj->textureIndex]->type == 2) {
						glMultiTexCoord2f(GL_TEXTURE0, point[0], point[1]);
						glMultiTexCoord2f(GL_TEXTURE1, point[0], point[1]);
					}
					else if (textures[obj->textureIndex]->type == 3) {
						glTexCoord2f(point[0], point[1]);
					}
				}

				//printf("Sending normal: (%f, %f, %f)\n", model->nList[face[k].n].ptr[0], model->nList[face[k].n].ptr[1], model->nList[face[k].n].ptr[2]);
				glNormal3fv(model->nList[face[k].n].ptr);
				glVertex3fv(model->vList[face[k].v].ptr);
			}
			glEnd();

			if (obj->textureIndex != -1)
				textures[obj->textureIndex]->detach();
		}

		glPopMatrix();
	}

	void renderMirror(int reflectionPass) {
		for (int i = 0; i < objects.size(); ++i) {
			auto obj = objects[i];
			if (modelNames[obj->modelIndex] == "Mirror.obj") {
				renderObject(obj, reflectionPass);
			}
		}
	}

	void renderNotMirror(int reflectionPass) {
		for (int i = 0; i < objects.size(); ++i) {
			if(modelNames[objects[i]->modelIndex] != "Mirror.obj")
				renderObject(objects[i], reflectionPass);
		}
	}

	void renderScene() {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL); 

		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5f);

		glEnable(GL_STENCIL_TEST);
		glClearStencil(0);
		glClearAccum(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

		glEnable(GL_CULL_FACE);

		camera->setProjection();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		camera->setViewMatrix(0);
		glRotatef(sceneRotation, camera->up[0], camera->up[1], camera->up[2]);

		if (isRenderNormal) {
			shaderNormal->use();
			renderNotMirror(0);
		}

		shader->use();
		renderNotMirror(0);

		glutSwapBuffers();
	}

	void reshapeWindow(int w, int h) {
		camera->reshape(w, h);
	}

	void zoomIn(int step) {
		camera->zoomIn(step);
	}

	void moveLeft(int step) {
		sceneRotation += step*10;
	}

	void toggleShowNormal() {
		isRenderNormal = !isRenderNormal;
	}

	int getWindowWidth() {
		return camera->viewport[2];
	}

	int getWindowHeight() {
		return camera->viewport[3];
	}

	void setLighting() {
		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);

		for (int i = 0; i < lightCollection->lights.size(); ++i) {
			float finalPos[4];
			memcpy(finalPos, lightCollection->lights[i].pos, sizeof(float) * 4);
			finalPos[1] += lightYOffset;

			glEnable(GL_LIGHT0 + i);
			glLightfv(GL_LIGHT0 + i, GL_POSITION, finalPos);
			glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, lightCollection->lights[i].diffuse);
			glLightfv(GL_LIGHT0 + i, GL_SPECULAR, lightCollection->lights[i].specular);
			glLightfv(GL_LIGHT0 + i, GL_AMBIENT, lightCollection->lights[i].ambient);
		}

		glEnable(GL_LIGHT_MODEL_AMBIENT);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightCollection->env_ambient);
	}

	void setCurrentObjectIndex(int idx) {
		if (idx >= 0 && idx < objects.size())
			currentObjectIndex = idx;
	}

	void setFocus(float x, float y, float z) {
		camera->setTarget(x, y, z);
	}

	void panObject(int x, int y) {
		if (currentObjectIndex < objects.size()) {
			objects[currentObjectIndex]->translation.x += x / 100.0f;
			objects[currentObjectIndex]->translation.y += y / 100.0f;
		}
	}

	void addLightY(int step) {
		lightYOffset += step/1.0f;
	}

	vector<mesh*> models;
	vector<string> modelNames;

	vector<Object*> objects;

	LightCollection *lightCollection;
	int currentObjectIndex = 0;
	Camera *camera;

	vector<Texture*> textures;
	float sceneRotation = 0;
	float lightYOffset = 0;
	bool isRenderNormal = true;

	Shader *shader;
	Shader *shaderNormal;
};

