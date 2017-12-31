#pragma once
#include <string>
#include "FreeImage.h"

using namespace std;

//type: 0.no texture, 1.single, 2.multi, 3.cube map
class Texture {
public:
	virtual void use() = 0;
	virtual void detach() = 0;
	int type;
	unsigned int id;
};

class SingleTexture: public Texture {
public:
	SingleTexture(string filename) {
		type = 1;

		glGenTextures(1, &id);

		FreeImage_Initialise();
		FIBITMAP* pImage = FreeImage_Load(FreeImage_GetFileType(filename.c_str(), 0), filename.c_str());
		FIBITMAP* p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
		int width = FreeImage_GetWidth(p32BitsImage);
		int height = FreeImage_GetHeight(p32BitsImage);

		glBindTexture(GL_TEXTURE_2D, id);
		glEnable(GL_TEXTURE_2D);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
			0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		FreeImage_Unload(p32BitsImage);
		FreeImage_Unload(pImage);
		FreeImage_DeInitialise();
	}

	~SingleTexture() {
		glDeleteTextures(1, &id);
	}

	void use() {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

	void detach() {
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

class MultiTexture : public Texture {
public:
	MultiTexture(string filename1, string filename2) {
		type = 2;

		glGenTextures(1, &id1);
		glGenTextures(1, &id2);

		FreeImage_Initialise();

		//Load texture 0
		FIBITMAP* pImage = FreeImage_Load(FreeImage_GetFileType(filename1.c_str(), 0), filename1.c_str());
		FIBITMAP* p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
		int width = FreeImage_GetWidth(p32BitsImage);
		int height = FreeImage_GetHeight(p32BitsImage);

		glBindTexture(GL_TEXTURE_2D, id1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
			0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		FreeImage_Unload(p32BitsImage);
		FreeImage_Unload(pImage);

		//Load texture 1
		pImage = FreeImage_Load(FreeImage_GetFileType(filename2.c_str(), 0), filename2.c_str());
		p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
		width = FreeImage_GetWidth(p32BitsImage);
		height = FreeImage_GetHeight(p32BitsImage);

		glBindTexture(GL_TEXTURE_2D, id2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
			0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		FreeImage_Unload(p32BitsImage);
		FreeImage_Unload(pImage);

		FreeImage_DeInitialise();
	}

	~MultiTexture() {
		glDeleteTextures(1, &id2);
		glDeleteTextures(1, &id1);
	}

	void use() {
		//bind texture 0
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, id1);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

		//bind texture 1
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, id2);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	}

	void detach() {
		//note the order!!!
		//unbind texture 1
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		//unbind texture 0
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	unsigned int id1, id2;
};

class CubeMap : public Texture {
public :
	CubeMap(const vector<string>& filenames) {
		type = 3;

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);

		FIBITMAP* pImage;
		FIBITMAP* p32BitsImage;
		for (int i = 0; i < 6; ++i) {
			FreeImage_Initialise();
			pImage = FreeImage_Load(FreeImage_GetFileType(filenames[i].c_str(), 0), filenames[i].c_str());
			p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
			int width = FreeImage_GetWidth(p32BitsImage);
			int height = FreeImage_GetHeight(p32BitsImage);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA,
				width, height, 0, GL_BGR, GL_UNSIGNED_BYTE,
				(void*)FreeImage_GetBits(pImage));
		}

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		FreeImage_Unload(p32BitsImage);
		FreeImage_Unload(pImage);
	}

	~CubeMap() {
		glDeleteTextures(1, &id);
	}

	void use() {
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

	void detach() {
		glDisable(GL_TEXTURE_CUBE_MAP);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_S);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
};