/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_MODEL_H
#define GRIM_MODEL_H

#include "common/memstream.h"
#include "engines/grim/object.h"
#include "graphics/matrix4.h"

namespace Grim {

class TextSplitter;
class Material;
class Mesh;
class ModelNode;
class CMap;

struct Sprite {
	void draw() const;

	Graphics::Vector3d _pos;
	float _width;
	float _height;
	bool _visible;
	Material *_material;
	Sprite *_next;
};

class Model : public Object {
public:
	// Construct a 3D model from the given data.
	Model(const Common::String &filename, const char *data, int len, CMap *cmap, Model *parent = NULL);
	void loadBinary(const char *&data, CMap *cmap);
	void loadText(TextSplitter *ts, CMap *cmap);
	void loadEMI(Common::MemoryReadStream &ms);
	void reload(CMap *cmap);
	void draw() const;
	Material *findMaterial(const char *name, CMap *cmap) const;

	~Model();

	Common::String _fname;
	ObjectPtr<CMap> _cmap;

	ModelNode *copyHierarchy();
	int getNumNodes() const { return _numHierNodes; }

//private:
	struct Geoset {
		void loadBinary(const char *&data, Material *materials[]);
		void loadText(TextSplitter *ts, Material *materials[]);
		void changeMaterials(Material *materials[]);
		Geoset() : _numMeshes(0) { }
		~Geoset();

		int _numMeshes;
		Mesh *_meshes;
	};

	void loadMaterial(int index, CMap *cmap);

	Model *_parent;
	int _numMaterials;
	char (*_materialNames)[32];
	Material **_materials;
	bool *_materialsShared;
	Graphics::Vector3d _insertOffset;
	int _numGeosets;
	Geoset *_geosets;
	float _radius;
	int _numHierNodes;
	ModelNode *_rootHierNode;
	Graphics::Vector3d _bboxPos;
	Graphics::Vector3d _bboxSize;
};

class MeshFace {
public:
	int loadBinary(const char *&data, Material *materials[]);
	void draw(float *vertices, float *vertNormals, float *textureVerts) const;
	void changeMaterial(Material *material);
	~MeshFace();

	Material *_material;
	int _type, _geo, _light, _tex;
	float _extraLight;
	int _numVertices;
	int *_vertices, *_texVertices;
	Graphics::Vector3d _normal;
};

class Mesh {
public:
	void loadBinary(const char *&data, Material *materials[]);
	void loadText(TextSplitter *ts, Material *materials[]);
	void changeMaterials(Material *materials[]);
	void draw(int *x1, int *y1, int *x2, int *y2) const;
	void update();
	Mesh() : _numFaces(0) { }
	~Mesh();

	char _name[32];
	float _radius;
	int _shadow, _geometryMode, _lightingMode, _textureMode;

	int _numVertices;
	int *_materialid;
	float *_vertices;		// sets of 3
	float *_verticesI;
	float *_vertNormals;	// sets of 3

	int _numTextureVerts;
	float *_textureVerts;	// sets of 2

	int _numFaces;
	MeshFace *_faces;
	Graphics::Matrix4 _matrix;
};

class ModelNode {
public:
	ModelNode() : _initialized(false) { }
	~ModelNode();
	void loadBinary(const char *&data, ModelNode *hierNodes, const Model::Geoset *g);
	void draw(int *x1, int *y1, int *x2, int *y2) const;
	void addChild(ModelNode *child);
	void removeChild(ModelNode *child);
	void setMatrix(Graphics::Matrix4 matrix);
	void update();
	void addSprite(Sprite *sprite);
	void removeSprite(Sprite *sprite);

	char _name[64];
	Mesh *_mesh;
	int _flags, _type;
	int _depth, _numChildren;
	ModelNode *_parent, *_child, *_sibling;
	Graphics::Vector3d _pos, _pivot;
	float _pitch, _yaw, _roll;
	Graphics::Vector3d _animPos;
	float _animPitch, _animYaw, _animRoll;
	bool _meshVisible, _hierVisible;
	bool _initialized;
	Graphics::Matrix4 _matrix;
	Graphics::Matrix4 _localMatrix;
	Graphics::Matrix4 _pivotMatrix;
	Sprite* _sprite;
};

} // end of namespace Grim

#endif
