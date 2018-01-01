#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "GL/glew.h"

using namespace std;

class Shader {
public:
	Shader(string vsFile, string fsFile, string gsFile = "") {
		string vsSource = readEntireFile(vsFile);
		const char *pVSSource = vsSource.c_str();
		int vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, &pVSSource, NULL);
		glCompileShader(vertShader);
		
		string fsSource = readEntireFile(fsFile);
		const char *pFSSource = fsSource.c_str();
		int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &pFSSource, NULL);
		glCompileShader(fragShader);

		int geomShader = 0;
		if (gsFile != "") {
			string gsSource = readEntireFile(gsFile);
			const char *pGSSource = gsSource.c_str();
			geomShader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geomShader, 1, &pGSSource, NULL);
			glCompileShader(geomShader);
		}

		program = glCreateProgram();
		glAttachShader(program, vertShader);
		glAttachShader(program, fragShader);
		if (gsFile != "")
			glAttachShader(program, geomShader);
		glLinkProgram(program);

		printShaderInfoLog(vertShader);
		printShaderInfoLog(fragShader);
		printShaderInfoLog(geomShader);
		printProgramInfoLog(program);
	}

	void use() {
		glUseProgram(program);
	}

	void setUniform(char *name, int value) {
		int location = glGetUniformLocation(program, name);
		glUniform1i(location, value);
	}

	void setUniform(char *name, float value) {
		int location = glGetUniformLocation(program, name);
		glUniform1f(location, value);
	}

	void setUniform(char *name, float *value) {
		int location = glGetUniformLocation(program, name);
		glUniform4fv(location, 1, value);
	}

	void setUniformMat4(char *name, float *value) {
		int location = glGetUniformLocation(program, name);
		glUniformMatrix4fv(location, 1, GL_FALSE, value);
	}

	string readEntireFile(string fileName) {
		ifstream file(fileName);
		stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	void printShaderInfoLog(GLuint obj)
	{
		int infologLength = 0;
		int charsWritten = 0;
		char *infoLog;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 1)
		{
			infoLog = new char[infologLength];
			glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
			printf("[ERROR]Failed to compile shader: %s\n", infoLog);
			delete[] infoLog;
		}
		else {
			printf("Succeed to compile shader\n");
		}
	}

	void printProgramInfoLog(GLuint obj)
	{
		int infologLength = 0;
		int charsWritten = 0;
		char *infoLog;
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 1)
		{
			infoLog = (char *)malloc(infologLength);
			glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
			printf("[ERROR]Failed to link program: %s\n", infoLog);
			free(infoLog);
		}
		else {
			printf("Succeed to link program\n");
		}
	}

	int program;
};