#pragma once
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <string.h>

//-------------------------------------------------------------------------------

#include "cyGL.h";
#include "cyVector.h";
#include "cyTriMesh.h";

//-------------------------------------------------------------------------------

#include "lodepng.h";

//-------------------------------------------------------------------------------

#include "xwHelper.h";

//-------------------------------------------------------------------------------

class Material {
public:
	struct Texture {
		std::vector<unsigned char> textureData;
		unsigned width;
		unsigned height;
		lodepng::State state;

		//! Constructor
		Texture() {}
		Texture(const char* n) {
			char objPath[50] = "Data\\";
			strcat(objPath, n);
			std::vector<unsigned char> png;
			unsigned error = lodepng::load_file(png, objPath);
			if (!error) error = lodepng::decode(textureData, width, height, state, png);
			if (error) printf("error: %s\n", lodepng_error_text(error));
		}
		~Texture() { textureData.~vector(); }
	};
protected:
	Texture diffuseTexture;
	Texture specularTexture;
public:
	void Initialize(cyTriMesh::Mtl* mtl);
	Texture GetDiffuseTextureData() { return diffuseTexture; }
	Texture GetSpecularTextureData() { return specularTexture; }
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

inline void Material::Initialize(cyTriMesh::Mtl* mtl) {
	//printf("Initialize material object!\n");
	//printf("has mtl: %s\n", mtl->name);
	//printf("Diffuse texture map: %s\n", mtl->map_Kd.data);
	//printf("Diffuse color: %f, %f, %f\n", mtl->Kd[0], mtl->Kd[1], mtl->Kd[2]);
	//printf("Specular texture map: %s\n", mtl->map_Ks.data);
	//printf("Specular color: %f, %f, %f\n", mtl->Ks[0], mtl->Ks[1], mtl->Ks[2]);
	//printf("Bump texture map: %s\n", mtl->map_bump.data);
	//printf("Illumination model: %d\n", mtl->Ni);

	diffuseTexture = Texture(mtl->map_Kd.data);
	specularTexture = Texture(mtl->map_Ks.data);
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------