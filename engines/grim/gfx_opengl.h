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

#ifndef GRIM_GFX_OPENGL_H
#define GRIM_GFX_OPENGL_H

#include "engines/grim/gfx_base.h"

#ifdef USE_OPENGL

#if defined (SDL_BACKEND) && !defined(__amigaos4__)
#include <SDL_opengl.h>
#undef ARRAYSIZE
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace Grim {

class ModelNode;
class Mesh;
class MeshFace;

class GfxOpenGL : public GfxBase {
public:
	GfxOpenGL();
	virtual ~GfxOpenGL();

	byte *setupScreen(int screenW, int screenH, bool fullscreen);
	void initExtensions();

	const char *getVideoDeviceName();

	void setupCamera(float fov, float nclip, float fclip, float roll);
	void positionCamera(Graphics::Vector3d pos, Graphics::Vector3d interest);

	void clearScreen();
	void flipBuffer();

	bool isHardwareAccelerated();

	void getBoundingBoxPos(const Mesh *model, int *x1, int *y1, int *x2, int *y2);

	void startActorDraw(Graphics::Vector3d pos, float scale, float yaw, float pitch, float roll);
	void finishActorDraw();
	void setShadow(Shadow *shadow);
	void drawShadowPlanes();
	void setShadowMode();
	void clearShadowMode();
	void setShadowColor(byte r, byte g, byte b);
	void getShadowColor(byte *r, byte *g, byte *b);

	void set3DMode();

	void translateViewpointStart(Graphics::Vector3d pos, float pitch, float yaw, float roll);
	void translateViewpointFinish();

	void drawHierachyNode(const ModelNode *node, int *x1, int *y1, int *x2, int *y2);
	void drawModelFace(const MeshFace *face, float *vertices, float *vertNormals, float *textureVerts);
	void drawSprite(const Sprite *sprite);

	void enableLights();
	void disableLights();
	void setupLight(Light *light, int lightId);

	void createMaterial(Texture *material, const char *data, const CMap *cmap);
	void selectMaterial(const Texture *material);
	void destroyMaterial(Texture *material);

	void createBitmap(BitmapData *bitmap);
	void drawBitmap(const Bitmap *bitmap);
	void destroyBitmap(BitmapData *bitmap);

	void createFont(Font *font);
	void destroyFont(Font *font);

	void createTextObject(TextObject *text);
	void drawTextObject(TextObject *text);
	void destroyTextObject(TextObject *text);

	Bitmap *getScreenshot(int w, int h);
	void storeDisplay();
	void copyStoredToDisplay();
	void dimScreen();
	void dimRegion(int x, int y, int w, int h, float level);
	void irisAroundRegion(int x1, int y1, int x2, int y2);

	void drawEmergString(int x, int y, const char *text, const Color &fgColor);
	void loadEmergFont();

	void drawRectangle(PrimitiveObject *primitive);
	void drawLine(PrimitiveObject *primitive);
	void drawPolygon(PrimitiveObject *primitive);

	void prepareMovieFrame(int width, int height, byte *bitmap);
	void drawMovieFrame(int offsetX, int offsetY);
	void releaseMovieFrame();

protected:
	void drawDepthBitmap(int x, int y, int w, int h, char *data);
private:
	GLuint _emergFont;
	int _smushNumTex;
	GLuint *_smushTexIds;
	int _smushWidth;
	int _smushHeight;
	byte *_storedDisplay;
	bool _useDepthShader;
	GLuint _fragmentProgram;
};

} // end of namespace Grim

#endif

#endif
