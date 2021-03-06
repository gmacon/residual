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

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetc
#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_getwd
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_stdin

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one... - this is needed before including util.h
#undef ARRAYSIZE
#endif

#include "common/archive.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/config-manager.h"

#include "gui/error.h"
#include "gui/gui-manager.h"

#include "engines/engine.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/lua.h"
#include "engines/grim/actor.h"
#include "engines/grim/movie/movie.h"
#include "engines/grim/savegame.h"
#include "engines/grim/registry.h"
#include "engines/grim/resource.h"
#include "engines/grim/localize.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/lab.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/primitives.h"
#include "engines/grim/objectstate.h"
#include "engines/grim/scene.h"

#include "engines/grim/lua/lualib.h"

#include "engines/grim/imuse/imuse.h"

#include "lua/lobject.h"
#include "lua/lstate.h"

namespace Grim {

static bool g_lua_initialized = false;

// Entries in the system.controls table

const ControlDescriptor controls[] = {
	{ "KEY_ESCAPE", Common::KEYCODE_ESCAPE },
	{ "KEY_1", Common::KEYCODE_1 },
	{ "KEY_2", Common::KEYCODE_2 },
	{ "KEY_3", Common::KEYCODE_3 },
	{ "KEY_4", Common::KEYCODE_4 },
	{ "KEY_5", Common::KEYCODE_5 },
	{ "KEY_6", Common::KEYCODE_6 },
	{ "KEY_7", Common::KEYCODE_7 },
	{ "KEY_8", Common::KEYCODE_8 },
	{ "KEY_9", Common::KEYCODE_9 },
	{ "KEY_0", Common::KEYCODE_0 },
	{ "KEY_MINUS", Common::KEYCODE_MINUS },
	{ "KEY_EQUALS", Common::KEYCODE_EQUALS },
	{ "KEY_BACK", Common::KEYCODE_BACKSPACE },
	{ "KEY_TAB", Common::KEYCODE_TAB },
	{ "KEY_Q", Common::KEYCODE_q },
	{ "KEY_W", Common::KEYCODE_w },
	{ "KEY_E", Common::KEYCODE_e },
	{ "KEY_R", Common::KEYCODE_r },
	{ "KEY_T", Common::KEYCODE_t },
	{ "KEY_Y", Common::KEYCODE_y },
	{ "KEY_U", Common::KEYCODE_u },
	{ "KEY_I", Common::KEYCODE_i },
	{ "KEY_O", Common::KEYCODE_o },
	{ "KEY_P", Common::KEYCODE_p },
	{ "KEY_LBRACKET", Common::KEYCODE_LEFTBRACKET },
	{ "KEY_RBRACKET", Common::KEYCODE_RIGHTBRACKET },
	{ "KEY_RETURN", Common::KEYCODE_RETURN },
	{ "KEY_LCONTROL", Common::KEYCODE_LCTRL },
	{ "KEY_A", Common::KEYCODE_a },
	{ "KEY_S", Common::KEYCODE_s },
	{ "KEY_D", Common::KEYCODE_d },
	{ "KEY_F", Common::KEYCODE_f },
	{ "KEY_G", Common::KEYCODE_g },
	{ "KEY_H", Common::KEYCODE_h },
	{ "KEY_J", Common::KEYCODE_j },
	{ "KEY_K", Common::KEYCODE_k },
	{ "KEY_L", Common::KEYCODE_l },
	{ "KEY_SEMICOLON", Common::KEYCODE_SEMICOLON },
	{ "KEY_APOSTROPHE", Common::KEYCODE_QUOTEDBL },
	{ "KEY_GRAVE", Common::KEYCODE_BACKQUOTE },
	{ "KEY_LSHIFT", Common::KEYCODE_LSHIFT },
	{ "KEY_BACKSLASH", Common::KEYCODE_BACKSLASH },
	{ "KEY_Z", Common::KEYCODE_z },
	{ "KEY_X", Common::KEYCODE_x },
	{ "KEY_C", Common::KEYCODE_c },
	{ "KEY_V", Common::KEYCODE_v },
	{ "KEY_B", Common::KEYCODE_b },
	{ "KEY_N", Common::KEYCODE_n },
	{ "KEY_M", Common::KEYCODE_m },
	{ "KEY_COMMA", Common::KEYCODE_COMMA },
	{ "KEY_PERIOD", Common::KEYCODE_PERIOD },
	{ "KEY_SLASH", Common::KEYCODE_SLASH },
	{ "KEY_RSHIFT", Common::KEYCODE_RSHIFT },
	{ "KEY_MULTIPLY", Common::KEYCODE_ASTERISK },
	{ "KEY_LMENU", Common::KEYCODE_LALT },
	{ "KEY_SPACE", Common::KEYCODE_SPACE },
	{ "KEY_CAPITAL", Common::KEYCODE_CAPSLOCK },
	{ "KEY_F1", Common::KEYCODE_F1 },
	{ "KEY_F2", Common::KEYCODE_F2 },
	{ "KEY_F3", Common::KEYCODE_F3 },
	{ "KEY_F4", Common::KEYCODE_F4 },
	{ "KEY_F5", Common::KEYCODE_F5 },
	{ "KEY_F6", Common::KEYCODE_F6 },
	{ "KEY_F7", Common::KEYCODE_F7 },
	{ "KEY_F8", Common::KEYCODE_F8 },
	{ "KEY_F9", Common::KEYCODE_F9 },
	{ "KEY_F10", Common::KEYCODE_F10 },
	{ "KEY_NUMLOCK", Common::KEYCODE_NUMLOCK },
	{ "KEY_SCROLL", Common::KEYCODE_SCROLLOCK },
	{ "KEY_NUMPAD7", Common::KEYCODE_KP7 },
	{ "KEY_NUMPAD8", Common::KEYCODE_KP8 },
	{ "KEY_NUMPAD9", Common::KEYCODE_KP9 },
	{ "KEY_SUBTRACT", Common::KEYCODE_KP_MINUS },
	{ "KEY_NUMPAD4", Common::KEYCODE_KP4 },
	{ "KEY_NUMPAD5", Common::KEYCODE_KP5 },
	{ "KEY_NUMPAD6", Common::KEYCODE_KP6 },
	{ "KEY_ADD", Common::KEYCODE_KP_PLUS },
	{ "KEY_NUMPAD1", Common::KEYCODE_KP1 },
	{ "KEY_NUMPAD2", Common::KEYCODE_KP2 },
	{ "KEY_NUMPAD3", Common::KEYCODE_KP3 },
	{ "KEY_NUMPAD0", Common::KEYCODE_KP0 },
	{ "KEY_DECIMAL", Common::KEYCODE_KP_PERIOD },
	{ "KEY_F11", Common::KEYCODE_F11 },
	{ "KEY_F12", Common::KEYCODE_F12 },
	{ "KEY_F13", Common::KEYCODE_F13 },
	{ "KEY_F14", Common::KEYCODE_F14 },
	{ "KEY_F15", Common::KEYCODE_F15 },
	{ "KEY_NUMPADEQUALS", Common::KEYCODE_KP_EQUALS },
	{ "KEY_AT", Common::KEYCODE_AT },
	{ "KEY_COLON", Common::KEYCODE_COLON },
	{ "KEY_UNDERLINE", Common::KEYCODE_UNDERSCORE },
	{ "KEY_STOP", Common::KEYCODE_BREAK },
	{ "KEY_NUMPADENTER", Common::KEYCODE_KP_ENTER },
	{ "KEY_RCONTROL", Common::KEYCODE_RCTRL },
	{ "KEY_NUMPADCOMMA", Common::KEYCODE_KP_PERIOD },
	{ "KEY_DIVIDE", Common::KEYCODE_KP_DIVIDE },
	{ "KEY_SYSRQ", Common::KEYCODE_SYSREQ },
	{ "KEY_RMENU", Common::KEYCODE_RALT },
	{ "KEY_HOME", Common::KEYCODE_HOME },
	{ "KEY_UP", Common::KEYCODE_UP },
	{ "KEY_PRIOR", Common::KEYCODE_PAGEUP },
	{ "KEY_LEFT", Common::KEYCODE_LEFT },
	{ "KEY_RIGHT", Common::KEYCODE_RIGHT },
	{ "KEY_END", Common::KEYCODE_END },
	{ "KEY_DOWN", Common::KEYCODE_DOWN },
	{ "KEY_NEXT", Common::KEYCODE_PAGEDOWN },
	{ "KEY_INSERT", Common::KEYCODE_INSERT },
	{ "KEY_DELETE", Common::KEYCODE_DELETE },
	{ "KEY_LWIN", Common::KEYCODE_LSUPER },
	{ "KEY_RWIN", Common::KEYCODE_RSUPER },
	{ "KEY_APPS", Common::KEYCODE_MENU },
	{ "KEY_EQUAL", Common::KEYCODE_EQUALS },
	{ "KEY_CONTROL", Common::KEYCODE_LCTRL },
	{ "KEY_SHIFT", Common::KEYCODE_LSHIFT },
	{ "KEY_ALT", Common::KEYCODE_LALT },
	{ "KEY_BACKSPACE", Common::KEYCODE_BACKSPACE },
	{ "KEY_NP9", Common::KEYCODE_KP9 },
	{ "KEY_NP8", Common::KEYCODE_KP8 },
	{ "KEY_NP7", Common::KEYCODE_KP7 },
	{ "KEY_NP6", Common::KEYCODE_KP6 },
	{ "KEY_NP5", Common::KEYCODE_KP5 },
	{ "KEY_NP4", Common::KEYCODE_KP4 },
	{ "KEY_NP3", Common::KEYCODE_KP3 },
	{ "KEY_NP2", Common::KEYCODE_KP2 },
	{ "KEY_NP1", Common::KEYCODE_KP1 },
	{ "KEY_NP0", Common::KEYCODE_KP0 },
	{ "KEY_NPENTER", Common::KEYCODE_KP_ENTER },
	{ "KEY_PAGEUP", Common::KEYCODE_PAGEUP },
	{ "KEY_PAGEDOWN", Common::KEYCODE_PAGEDOWN },
	{ "KEY_SCROLL", Common::KEYCODE_SCROLLOCK },
	{ "KEY_CAPSLOCK", Common::KEYCODE_CAPSLOCK },
	{ "KEY_LEFTBRACKET", Common::KEYCODE_LEFTBRACKET },
	{ "KEY_RIGHTBRACKET", Common::KEYCODE_RIGHTBRACKET },
	{ "KEY_TILDE", Common::KEYCODE_TILDE },
	{ "KEY_QUOTE", Common::KEYCODE_QUOTE },
	{ "KEY_PAUSE", Common::KEYCODE_PAUSE },
	{ "KEY_CLEAR", Common::KEYCODE_CLEAR },

	{ "KEY_JOY1_B1", KEYCODE_JOY1_B1 },
	{ "KEY_JOY1_B2", KEYCODE_JOY1_B2 },
	{ "KEY_JOY1_B3", KEYCODE_JOY1_B3 },
	{ "KEY_JOY1_B4", KEYCODE_JOY1_B4 },
	{ "KEY_JOY1_B5", KEYCODE_JOY1_B5 },
	{ "KEY_JOY1_B6", KEYCODE_JOY1_B6 },
	{ "KEY_JOY1_B7", KEYCODE_JOY1_B7 },
	{ "KEY_JOY1_B8", KEYCODE_JOY1_B8 },
	{ "KEY_JOY1_B9", KEYCODE_JOY1_B9 },
	{ "KEY_JOY1_B10", KEYCODE_JOY1_B10 },
	{ "KEY_JOY1_B11", KEYCODE_JOY1_B11 },
	{ "KEY_JOY1_B12", KEYCODE_JOY1_B12 },
	{ "KEY_JOY1_B13", KEYCODE_JOY1_B13 },
	{ "KEY_JOY1_B14", KEYCODE_JOY1_B14 },
	{ "KEY_JOY1_B15", KEYCODE_JOY1_B15 },
	{ "KEY_JOY1_B16", KEYCODE_JOY1_B16 },
	{ "KEY_JOY1_B17", KEYCODE_JOY1_B17 },
	{ "KEY_JOY1_B18", KEYCODE_JOY1_B18 },
	{ "KEY_JOY1_B19", KEYCODE_JOY1_B19 },
	{ "KEY_JOY1_B20", KEYCODE_JOY1_B20 },
	{ "KEY_JOY1_HLEFT", KEYCODE_JOY1_HLEFT },
	{ "KEY_JOY1_HUP", KEYCODE_JOY1_HUP },
	{ "KEY_JOY1_HRIGHT", KEYCODE_JOY1_HRIGHT },
	{ "KEY_JOY1_HDOWN", KEYCODE_JOY1_HDOWN },
	{ "KEY_JOY2_B1", KEYCODE_JOY2_B1 },
	{ "KEY_JOY2_B2", KEYCODE_JOY2_B2 },
	{ "KEY_JOY2_B3", KEYCODE_JOY2_B3 },
	{ "KEY_JOY2_B4", KEYCODE_JOY2_B4 },
	{ "KEY_JOY2_B5", KEYCODE_JOY2_B5 },
	{ "KEY_JOY2_B6", KEYCODE_JOY2_B6 },
	{ "KEY_JOY2_B7", KEYCODE_JOY2_B7 },
	{ "KEY_JOY2_B8", KEYCODE_JOY2_B8 },
	{ "KEY_JOY2_B9", KEYCODE_JOY2_B9 },
	{ "KEY_JOY2_B10", KEYCODE_JOY2_B10 },
	{ "KEY_JOY2_HLEFT", KEYCODE_JOY1_HLEFT },
	{ "KEY_JOY2_HUP", KEYCODE_JOY2_HUP },
	{ "KEY_JOY2_HRIGHT", KEYCODE_JOY2_HRIGHT },
	{ "KEY_JOY2_HDOWN", KEYCODE_JOY2_HDOWN },
	{ "KEY_MOUSE_B1", KEYCODE_MOUSE_B1 },
	{ "KEY_MOUSE_B2", KEYCODE_MOUSE_B2 },
	{ "KEY_MOUSE_B3", KEYCODE_MOUSE_B3 },
	{ "KEY_MOUSE_B4", KEYCODE_MOUSE_B4 },
	{ "AXIS_JOY1_X", KEYCODE_AXIS_JOY1_X },
	{ "AXIS_JOY1_Y", KEYCODE_AXIS_JOY1_Y },
	{ "AXIS_JOY1_Z", KEYCODE_AXIS_JOY1_Z },
	{ "AXIS_JOY1_R", KEYCODE_AXIS_JOY1_R },
	{ "AXIS_JOY1_U", KEYCODE_AXIS_JOY1_U },
	{ "AXIS_JOY1_V", KEYCODE_AXIS_JOY1_V },
	{ "AXIS_JOY2_X", KEYCODE_AXIS_JOY2_X },
	{ "AXIS_JOY2_Y", KEYCODE_AXIS_JOY2_Y },
	{ "AXIS_JOY2_Z", KEYCODE_AXIS_JOY2_Z },
	{ "AXIS_JOY2_R", KEYCODE_AXIS_JOY2_R },
	{ "AXIS_JOY2_U", KEYCODE_AXIS_JOY2_U },
	{ "AXIS_JOY2_V", KEYCODE_AXIS_JOY2_V },
	{ "AXIS_MOUSE_X", KEYCODE_AXIS_MOUSE_X },
	{ "AXIS_MOUSE_Y", KEYCODE_AXIS_MOUSE_Y },
	{ "AXIS_MOUSE_Z", KEYCODE_AXIS_MOUSE_Z },

//PS2
	{ "KEY_JOY1_SQUARE", KEYCODE_JOY1_B1 },
	{ "KEY_JOY1_TRIANGLE", KEYCODE_JOY1_B2 },
	{ "KEY_JOY1_CIRCLE", KEYCODE_JOY1_B3 },
	{ "KEY_JOY1_X", KEYCODE_JOY1_B4 },
	{ "KEY_JOY1_R1", KEYCODE_JOY1_B5 },
	{ "KEY_JOY1_L1", KEYCODE_JOY1_B6 },
	{ "KEY_JOY1_R2", KEYCODE_JOY1_B7 },
	{ "KEY_JOY1_L2", KEYCODE_JOY1_B8 },
	{ "KEY_JOY1_START", KEYCODE_JOY1_B9 },
	{ "KEY_JOY1_SELECT", KEYCODE_JOY1_B10 },
	{ "KEY_JOY1_DPAD_U", KEYCODE_JOY1_B11 },
	{ "KEY_JOY1_DPAD_D", KEYCODE_JOY1_B12 },
	{ "KEY_JOY1_DPAD_L", KEYCODE_JOY1_B13 },
	{ "KEY_JOY1_DPAD_R", KEYCODE_JOY1_B14 },
	{ "KEY_JOY1_LMUSHROOM", KEYCODE_JOY1_B15 },
	{ "KEY_JOY1_RMUSHROOM", KEYCODE_JOY1_B16 },
// Joy2
	{ "KEY_JOY2_SQUARE", KEYCODE_JOY2_B1 },
	{ "KEY_JOY2_TRIANGLE", KEYCODE_JOY2_B2 },
	{ "KEY_JOY2_CIRCLE", KEYCODE_JOY2_B3 },
	{ "KEY_JOY2_X", KEYCODE_JOY2_B4 },
	{ "KEY_JOY2_R1", KEYCODE_JOY2_B5 },
	{ "KEY_JOY2_L1", KEYCODE_JOY2_B6 },
	{ "KEY_JOY2_R2", KEYCODE_JOY2_B7 },
	{ "KEY_JOY2_L2", KEYCODE_JOY2_B8 },
	{ "KEY_JOY2_START", KEYCODE_JOY2_B9 },
	{ "KEY_JOY2_SELECT", KEYCODE_JOY2_B10 },
	{ "KEY_JOY2_DPAD_U", KEYCODE_JOY2_B11 },
	{ "KEY_JOY2_DPAD_D", KEYCODE_JOY2_B12 },
	{ "KEY_JOY2_DPAD_L", KEYCODE_JOY2_B13 },
	{ "KEY_JOY2_DPAD_R", KEYCODE_JOY2_B14 },
	{ "KEY_JOY2_LMUSHROOM", KEYCODE_JOY2_B15 },
	{ "KEY_JOY2_RMUSHROOM", KEYCODE_JOY2_B16 },

// tell EMI there is no joystick selected
	{ "joy_selected", -1 },

	{ NULL, 0 }
};

// CHAR_KEY tests to see whether a keycode is for
// a "character" handler or a "button" handler
#define CHAR_KEY(k) ((k >= 'a' && k <= 'z') || (k >= 'A' && k <= 'Z') || (k >= '0' && k <= '9') || k == ' ')

GrimEngine *g_grim = NULL;
GfxBase *g_driver = NULL;
int g_imuseState = -1;

GrimEngine::GrimEngine(OSystem *syst, uint32 gameFlags, GrimGameType gameType, Common::Platform platform, Common::Language language) :
		Engine(syst), _currScene(NULL), _selectedActor(NULL) {
	g_grim = this;

	_gameType = gameType;
	_gameFlags = gameFlags;
	_gamePlatform = platform;
	_gameLanguage = language;

	g_registry = new Registry();
	g_resourceloader = NULL;
	g_localizer = NULL;
	g_movie = NULL;
	g_imuse = NULL;

	_showFps = (tolower(g_registry->get("show_fps", "false")[0]) == 't');

#ifdef USE_OPENGL
	_softRenderer = (tolower(g_registry->get("soft_renderer", "false")[0]) == 't');
#else
	_softRenderer = true;
#endif

	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_currScene = NULL;
	_selectedActor = NULL;
	_talkingActor = NULL;
	_controlsEnabled = new bool[KEYCODE_EXTRA_LAST];
	_controlsState = new bool[KEYCODE_EXTRA_LAST];
	for (int i = 0; i < KEYCODE_EXTRA_LAST; i++) {
		_controlsEnabled[i] = false;
		_controlsState[i] = false;
	}
	_speechMode = TextAndVoice;
	_textSpeed = 7;
	_mode = _previousMode = ENGINE_MODE_NORMAL;
	_flipEnable = true;
	int speed = atol(g_registry->get("engine_speed", "30"));
	if (speed <= 0 || speed > 100)
		_speedLimitMs = 30;
	else
		_speedLimitMs = 1000 / speed;
	char buf[20];
	sprintf(buf, "%d", 1000 / _speedLimitMs);
	g_registry->set("engine_speed", buf);
	_refreshDrawNeeded = true;
	_listFilesIter = NULL;
	_savedState = NULL;
	_fps[0] = 0;
	_iris = new Iris();

	PoolColor *c = new PoolColor(0, 0, 0);
	new PoolColor(255, 255, 255); // Default color for actors. Id == 2

	_printLineDefaults.setX(0);
	_printLineDefaults.setY(100);
	_printLineDefaults.setWidth(0);
	_printLineDefaults.setHeight(0);
	_printLineDefaults.setFGColor(c);
	_printLineDefaults.setFont(NULL);
	_printLineDefaults.setJustify(TextObject::LJUSTIFY);
	_printLineDefaults.setDisabled(false);

	_sayLineDefaults.setX(0);
	_sayLineDefaults.setY(100);
	_sayLineDefaults.setWidth(0);
	_sayLineDefaults.setHeight(0);
	_sayLineDefaults.setFGColor(c);
	_sayLineDefaults.setFont(NULL);
	_sayLineDefaults.setJustify(TextObject::CENTER);
	_sayLineDefaults.setDisabled(false);

	_blastTextDefaults.setX(0);
	_blastTextDefaults.setY(200);
	_blastTextDefaults.setWidth(0);
	_blastTextDefaults.setHeight(0);
	_blastTextDefaults.setFGColor(c);
	_blastTextDefaults.setFont(NULL);
	_blastTextDefaults.setJustify(TextObject::LJUSTIFY);
	_blastTextDefaults.setDisabled(false);

	// Add 'movies' subdirectory for the demo
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "movies");
}

GrimEngine::~GrimEngine() {
	delete[] _controlsEnabled;
	delete[] _controlsState;

	Scene::getPool()->deleteObjects();
	Actor::getPool()->deleteObjects();
	PrimitiveObject::getPool()->deleteObjects();
	TextObject::getPool()->deleteObjects();
	Bitmap::getPool()->deleteObjects();
	Font::getPool()->deleteObjects();
	ObjectState::getPool()->deleteObjects();
	PoolColor::getPool()->deleteObjects();

	if (g_lua_initialized) {
		lua_removelibslists();
		lua_close();
		lua_iolibclose();
		g_lua_initialized = false;
	}
	if (g_registry) {
		g_registry->save();
		delete g_registry;
		g_registry = NULL;
	}
	delete g_movie;
	g_movie = NULL;
	delete g_imuse;
	g_imuse = NULL;
	delete g_localizer;
	g_localizer = NULL;
	delete g_resourceloader;
	g_resourceloader = NULL;
	delete g_driver;
	g_driver = NULL;
	delete _iris;
}

Common::Error GrimEngine::run() {
	g_resourceloader = new ResourceLoader();
	g_localizer = new Localizer();
	if (getGameType() == GType_GRIM)
		g_movie = CreateSmushPlayer();
	else if (getGameType() == GType_MONKEY4) {
		if (_gamePlatform == Common::kPlatformPS2)
			g_movie = CreateMpegPlayer();
		else
			g_movie = CreateBinkPlayer();
	}
	g_imuse = new Imuse(20);

	bool fullscreen = (tolower(g_registry->get("fullscreen", "false")[0]) == 't');

	if (!_softRenderer && !g_system->hasFeature(OSystem::kFeatureOpenGL)){
		warning("gfx backend doesn't support hardware rendering");
		_softRenderer=true;
	}

	if (_softRenderer)
		g_driver = CreateGfxTinyGL();
#ifdef USE_OPENGL
	else
		g_driver = CreateGfxOpenGL();
#endif

	g_driver->setupScreen(640, 480, fullscreen);

	// refresh the theme engine so that we can show the gui overlay without it crashing.
	GUI::GuiManager::instance().theme()->refresh();

	Bitmap *splash_bm;
	if (!(_gameFlags & ADGF_DEMO) && getGameType() == GType_GRIM)
		splash_bm = g_resourceloader->loadBitmap("splash.bm");
	else if ((_gameFlags & ADGF_DEMO) && getGameType() == GType_MONKEY4)
		splash_bm = g_resourceloader->loadBitmap("splash.til");

	g_driver->clearScreen();

	if (!(_gameFlags & ADGF_DEMO) && getGameType() == GType_GRIM)
		splash_bm->draw();
	else if ((_gameFlags & ADGF_DEMO) && getGameType() == GType_MONKEY4)
		splash_bm->draw();

	g_driver->flipBuffer();

	lua_iolibopen();
	lua_strlibopen();
	lua_mathlibopen();

	if (getGameType() == GType_GRIM) {
		registerGrimOpcodes();

		// FIXME/HACK: see PutActorInSet
		const char *func = "function reset_doorman() doorman_in_hot_box = FALSE end";
		lua_pushstring(func);
		lua_call("dostring");
	} else
		registerMonkeyOpcodes();

	registerLua();
	g_lua_initialized = true;

	bundle_dofile("_system.lua");

	lua_pushnil();		// resumeSave
	lua_pushnil();		// bootParam - not used in scripts
	lua_call("BOOT");

	_savegameLoadRequest = false;
	_savegameSaveRequest = false;


	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		int slot = ConfMan.getInt("save_slot");
		assert(slot <= 99);
		assert(slot >= 0);
		char saveName[16];
		sprintf(saveName, "grim%02d.gsv", slot);
		_savegameLoadRequest = true;
		_savegameFileName = saveName;
	}

	g_grim->setMode(ENGINE_MODE_NORMAL);
	delete splash_bm;
	g_grim->mainLoop();

	return Common::kNoError;
}

int GrimEngine::bundle_dofile(const char *filename) {
	Block *b = g_resourceloader->getFileBlock(filename);
	if (!b) {
		delete b;
		// Don't print warnings on Scripts\foo.lua,
		// d:\grimFandango\Scripts\foo.lua
		if (!strstr(filename, "Scripts\\") && (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL))
			warning("Cannot find script %s", filename);

		return 2;
	}

	int result = lua_dobuffer(const_cast<char *>(b->getData()), b->getLen(), const_cast<char *>(filename));
	delete b;
	return result;
}

int GrimEngine::single_dofile(const char *filename) {
	Common::File *f = new Common::File();

	if (!f->open(filename)) {
		delete f;
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Cannot find script %s", filename);

		return 2;
	}

	int32 size = f->size();
	char *data = new char[size];
	f->read(data, size);

	int result = lua_dobuffer(data, size, const_cast<char *>(filename));
	delete f;
	delete[] data;

	return result;
}

extern int refSystemTable;

void GrimEngine::handlePause() {
	lua_Object func;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("pauseHandler");
	lua_Object handler = lua_gettable();
	if (lua_istable(handler)) {
		lua_pushobject(handler);
		lua_pushstring("pauseHandler");
		func = lua_gettable();
		if (!lua_isfunction(func))
			error("handlePause: handler not a function");
		lua_pushobject(handler);
	} else if (lua_isfunction(handler)) {
		func = handler;
	} else if (!lua_isnil(handler)) {
		error("handlePause: invalid handler");
		return;
	} else {
		lua_endblock();
		return;
	}

	lua_callfunction(func);

	lua_endblock();
}

void GrimEngine::handleExit() {
	lua_Object func;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("exitHandler");
	lua_Object handler = lua_gettable();
	if (lua_istable(handler)) {
		lua_pushobject(handler);
		lua_pushstring("exitHandler");
		func = lua_gettable();
		if (!lua_isfunction(func))
			error("handleExit: handler not a function");
		lua_pushobject(handler);
	} else if (lua_isfunction(handler)) {
		func = handler;
	} else if (!lua_isnil(handler)) {
		error("handleExit: invalid handler");
		return;
	} else {
		lua_endblock();
		return;
	}

	lua_callfunction(func);

	lua_endblock();
}

void GrimEngine::handleUserPaint() {
	lua_Object func;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("userPaintHandler");
	lua_Object handler = lua_gettable();
	if (lua_istable(handler)) {
		lua_pushobject(handler);
		lua_pushstring("userPaintHandler");
		func = lua_gettable();
		if (!lua_isfunction(func))
			error("handleUserPaint: handler not a function");
		lua_pushobject(handler);
	} else if (lua_isfunction(handler)) {
		func = handler;
	} else if (!lua_isnil(handler)) {
		error("handleUserPaint: invalid handler");
		return;
	} else {
		lua_endblock();
		return;
	}

	lua_callfunction(func);

	lua_endblock();
}

void GrimEngine::handleChars(int operation, int key, int /*keyModifier*/, uint16 ascii) {
	lua_Object func;
	char keychar[2];

	if (!CHAR_KEY(ascii))
		return;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("characterHandler");
	lua_Object handler = lua_gettable();
	if (lua_istable(handler)) {
		lua_pushobject(handler);
		lua_pushstring("characterHandler");
		func = lua_gettable();
		if (!lua_isfunction(func))
			error("handleChars: handler not a function");
		lua_pushobject(handler);
	} else if (lua_isfunction(handler)) {
		func = handler;
	} else if (!lua_isnil(handler)) {
		error("handleChars: invalid handler");
		return;
	} else {
		lua_endblock();
		return;
	}

	keychar[0] = ascii;
	keychar[1] = 0;
	lua_pushstring(keychar);
	lua_callfunction(func);

	lua_endblock();
}

void GrimEngine::handleControls(int operation, int key, int /*keyModifier*/, uint16 ascii) {
	lua_Object buttonFunc, joyFunc;
	bool buttonFuncIsTable;

	// If we're not supposed to handle the key then don't
	if (!_controlsEnabled[key])
		return;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("buttonHandler");
	lua_Object buttonHandler = lua_gettable();
	if (lua_istable(buttonHandler)) {
		lua_pushobject(buttonHandler);
		lua_pushstring("buttonHandler");
		buttonFunc = lua_gettable();
		if (!lua_isfunction(buttonFunc)) {
			error("handleControls: button handler not a function");
			return;
		}
		buttonFuncIsTable = true;
	} else if (lua_isfunction(buttonHandler)) {
		buttonFunc = buttonHandler;
		buttonFuncIsTable = false;
	} else {
		error("handleControls: invalid keys handler");
		return;
	}

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("axisHandler");
	lua_Object joyHandler = lua_gettable();
	if (lua_istable(joyHandler)) {
		lua_pushobject(joyHandler);
		lua_pushstring("axisHandler");
		joyFunc = lua_gettable();
		if (!lua_isfunction(joyFunc)) {
			error("handleControls: joystick handler not a function");
			return;
		}
	} else if (lua_isfunction(joyHandler)) {
		joyFunc = joyHandler;
	} else {
		error("handleControls: invalid joystick handler");
		return;
	}
	if (buttonFuncIsTable)
		lua_pushobject(buttonHandler);
	lua_pushnumber(key);
	if (operation == Common::EVENT_KEYDOWN) {
		lua_pushnumber(1);
		lua_pushnumber(1);
	} else {
		lua_pushnil();
		lua_pushnumber(0);
	}
	lua_pushnumber(0);
	lua_callfunction(buttonFunc);

	if (operation == Common::EVENT_KEYDOWN)
		_controlsState[key] = true;
	else if (operation == Common::EVENT_KEYUP)
		_controlsState[key] = false;

	lua_endblock();
}

void GrimEngine::handleDebugLoadResource() {
	void *resource = NULL;
	int c, i = 0;
	char buf[512];

	// Tool for debugging the loading of a particular resource without
	// having to actually make it all the way to it in the game
	fprintf(stderr, "Enter resource to load (extension specifies type): ");
	while (i < 512 && (c = fgetc(stdin)) != EOF && c != '\n')
		buf[i++] = c;

	buf[i] = '\0';
	if (strstr(buf, ".key"))
		resource = (void *)g_resourceloader->loadKeyframe(buf);
	else if (strstr(buf, ".zbm") || strstr(buf, ".bm"))
		resource = (void *)g_resourceloader->loadBitmap(buf);
	else if (strstr(buf, ".cmp"))
		resource = (void *)g_resourceloader->loadColormap(buf);
	else if (strstr(buf, ".cos"))
		resource = (void *)g_resourceloader->loadCostume(buf, NULL);
	else if (strstr(buf, ".lip"))
		resource = (void *)g_resourceloader->loadLipSync(buf);
	else if (strstr(buf, ".snm"))
		resource = (void *)g_movie->play(buf, false, 0, 0);
	else if (strstr(buf, ".wav") || strstr(buf, ".imu")) {
		g_imuse->startSfx(buf);
		resource = (void *)1;
	} else if (strstr(buf, ".mat")) {
		CMap *cmap = g_resourceloader->loadColormap("item.cmp");
		warning("Default colormap applied to resources loaded in this fashion");
		resource = (void *)g_resourceloader->loadMaterial(buf, cmap);
	} else {
		warning("Resource type not understood");
	}
	if (!resource)
		warning("Requested resouce (%s) not found", buf);
}

static void cameraChangeHandle(int prev, int next) {
	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("camChangeHandler");
	lua_Object func = lua_gettable();

	if (lua_isfunction(func)) {
		lua_pushnumber(prev);
		lua_pushnumber(next);
		lua_callfunction(func);
	}

	lua_endblock();
}

static void cameraPostChangeHandle(int num) {
	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("postCamChangeHandler");
	lua_Object func = lua_gettable();

	if (lua_isfunction(func)) {
		lua_pushnumber(num);
		lua_callfunction(func);
	}

	lua_endblock();
}

void GrimEngine::drawPrimitives() {
	_iris->draw();

	// Draw text
	for (TextObject::Pool::Iterator i = TextObject::getPool()->getBegin();
		 i != TextObject::getPool()->getEnd(); ++i) {
		i->_value->draw();
	}
}

void GrimEngine::playIrisAnimation(Iris::Direction dir, int x, int y, int time) {
	_iris->play(dir, x, y, time);
}

void GrimEngine::luaUpdate() {
	if (_savegameLoadRequest || _savegameSaveRequest)
		return;

	// Update timing information
	unsigned newStart = g_system->getMillis();
	if (newStart < _frameStart) {
		_frameStart = newStart;
		return;
	}
	_frameTime = newStart - _frameStart;
	_frameStart = newStart;

	if (_mode == ENGINE_MODE_PAUSE || _shortFrame) {
		_frameTime = 0;
	}

	_frameTimeCollection += _frameTime;
	if (_frameTimeCollection > 10000) {
		_frameTimeCollection = 0;
		lua_collectgarbage(0);
	}

	lua_beginblock();
	setFrameTime(_frameTime);
	lua_endblock();

	lua_beginblock();
	setMovieTime(_movieTime);
	lua_endblock();

	// Run asynchronous tasks
	lua_runtasks();

	if (_currScene && (_mode == ENGINE_MODE_NORMAL || _mode == ENGINE_MODE_SMUSH)) {
		// Update the actors. Do it here so that we are sure to react asap to any change
		// in the actors state caused by lua.
		for (Actor::Pool::Iterator i = Actor::getPool()->getBegin(); i != Actor::getPool()->getEnd(); ++i) {
			Actor *a = i->_value;

			// Note that the actor need not be visible to update chores, for example:
			// when Manny has just brought Meche back he is offscreen several times
			// when he needs to perform certain chores
			if (a->isInSet(_currScene->getName()))
				a->update(_frameTime);
		}

		_iris->update(_frameTime);
	}

	for (TextObject::Pool::Iterator i = TextObject::getPool()->getBegin();
		 i != TextObject::getPool()->getEnd(); i++) {
		i->_value->update();
	}
}

void GrimEngine::updateDisplayScene() {
	_doFlip = true;

	if (_mode == ENGINE_MODE_SMUSH) {
		if (g_movie->isPlaying()) {
			_movieTime = g_movie->getMovieTime();
			if (g_movie->isUpdateNeeded()) {
				g_driver->prepareMovieFrame(g_movie->getWidth(), g_movie->getHeight(), g_movie->getDstPtr());
				g_movie->clearUpdateNeeded();
			}
			int frame = g_movie->getFrame();
			if (frame > 0) {
				if (frame != _prevSmushFrame) {
					_prevSmushFrame = g_movie->getFrame();
					g_driver->drawMovieFrame(g_movie->getX(), g_movie->getY());
					if (_showFps)
						g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));
				} else
					_doFlip = false;
			} else
				g_driver->releaseMovieFrame();
		}
		// Draw Primitives
		for (PrimitiveObject::Pool::Iterator i = PrimitiveObject::getPool()->getBegin();
			i != PrimitiveObject::getPool()->getEnd(); ++i) {
			i->_value->draw();
		}
		drawPrimitives();
	} else if (_mode == ENGINE_MODE_NORMAL) {
		if (!_currScene)
			return;

		cameraPostChangeHandle(_currScene->getSetup());

		g_driver->clearScreen();

		_prevSmushFrame = 0;
		_movieTime = 0;

		_currScene->drawBackground();

		// Draw underlying scene components
		// Background objects are drawn underneath everything except the background
		// There are a bunch of these, especially in the tube-switcher room
		_currScene->drawBitmaps(ObjectState::OBJSTATE_BACKGROUND);

		// Underlay objects are just above the background
		_currScene->drawBitmaps(ObjectState::OBJSTATE_UNDERLAY);

		// State objects are drawn on top of other things, such as the flag
		// on Manny's message tube
		_currScene->drawBitmaps(ObjectState::OBJSTATE_STATE);

		// Play SMUSH Animations
		// This should occur on top of all underlying scene objects,
		// a good example is the tube switcher room where some state objects
		// need to render underneath the animation or you can't see what's going on
		// This should not occur on top of everything though or Manny gets covered
		// up when he's next to Glottis's service room
		if (g_movie->isPlaying()) {
			_movieTime = g_movie->getMovieTime();
			if (g_movie->isUpdateNeeded()) {
				g_driver->prepareMovieFrame(g_movie->getWidth(), g_movie->getHeight(), g_movie->getDstPtr());
				g_movie->clearUpdateNeeded();
			}
			if (g_movie->getFrame() > 0)
				g_driver->drawMovieFrame(g_movie->getX(), g_movie->getY());
			else
				g_driver->releaseMovieFrame();
		}

		// Draw Primitives
		for (PrimitiveObject::Pool::Iterator i = PrimitiveObject::getPool()->getBegin();
			 i != PrimitiveObject::getPool()->getEnd(); ++i) {
			i->_value->draw();
		}

		_currScene->setupCamera();

		g_driver->set3DMode();

		_currScene->setupLights();

		// Draw actors
		for (Actor::Pool::Iterator i = Actor::getPool()->getBegin(); i != Actor::getPool()->getEnd(); ++i) {
			Actor *a = i->_value;
			if (a->isInSet(_currScene->getName()) && a->isVisible())
				a->draw();
			a->undraw(a->isInSet(_currScene->getName()) && a->isVisible());
		}
		flagRefreshShadowMask(false);

		// Draw overlying scene components
		// The overlay objects should be drawn on top of everything else,
		// including 3D objects such as Manny and the message tube
		_currScene->drawBitmaps(ObjectState::OBJSTATE_OVERLAY);

		drawPrimitives();
	} else if (_mode == ENGINE_MODE_DRAW) {
		_doFlip = false;
		_prevSmushFrame = 0;
		_movieTime = 0;
	}
}

void GrimEngine::doFlip() {
	if (_showFps && _doFlip)
		g_driver->drawEmergString(550, 25, _fps, Color(255, 255, 255));

	if (_doFlip && _flipEnable)
		g_driver->flipBuffer();

	if (_showFps && _doFlip && _mode != ENGINE_MODE_DRAW) {
		_frameCounter++;
		unsigned int currentTime = g_system->getMillis();
		unsigned int delta = currentTime - _lastFrameTime;
		if (delta > 500) {
			sprintf(_fps, "%7.2f", (double)(_frameCounter * 1000) / (double)delta );
			_frameCounter = 0;
			_lastFrameTime = currentTime;
		}
	}
}

void GrimEngine::mainLoop() {
	_movieTime = 0;
	_frameTime = 0;
	_frameStart = g_system->getMillis();
	_frameCounter = 0;
	_lastFrameTime = 0;
	_frameTimeCollection = 0;
	_prevSmushFrame = 0;
	_refreshShadowMask = false;
	_shortFrame = false;
	bool resetShortFrame = false;

	for (;;) {
		uint32 startTime = g_system->getMillis();
		if (_shortFrame) {
			if (resetShortFrame) {
				_shortFrame = false;
			}
			resetShortFrame = !resetShortFrame;
		}

		if (_savegameLoadRequest) {
			savegameRestore();
		}
		if (_savegameSaveRequest) {
			savegameSave();
		}

		g_imuse->flushTracks();
		g_imuse->refreshScripts();

		// Process events
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			// Handle any buttons, keys and joystick operations
			if (event.type == Common::EVENT_KEYDOWN) {
				if (_mode != ENGINE_MODE_DRAW && _mode != ENGINE_MODE_SMUSH && (event.kbd.ascii == 'q')) {
					handleExit();
					break;
				} else {
					handleChars(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
				}
			}
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP) {
				handleControls(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
			}
			// Check for "Hard" quit"
			if (event.type == Common::EVENT_QUIT)
				return;
			if (event.type == Common::EVENT_SCREEN_CHANGED)
				_refreshDrawNeeded = true;

			// Allow lua to react to the event.
			// Without this lua_update switching the entries in the menu is slow because
			// if the button is not kept pressed the KEYUP will arrive just after the KEYDOWN
			// and it will break the lua scripts that checks for the state of the button
			// with GetControlState()
			luaUpdate();
		}

		luaUpdate();

		if (_mode != ENGINE_MODE_PAUSE) {
			updateDisplayScene();
			doFlip();
		}

		if (g_imuseState != -1) {
			g_imuse->setMusicState(g_imuseState);
			g_imuseState = -1;
		}

		uint32 endTime = g_system->getMillis();
		if (startTime > endTime)
			continue;
		uint32 diffTime = endTime - startTime;
		if (_speedLimitMs == 0)
			continue;
		if (diffTime < _speedLimitMs) {
			uint32 delayTime = _speedLimitMs - diffTime;
			g_system->delayMillis(delayTime);
		}
	}
}

void GrimEngine::saveGame(const Common::String &file) {
	_savegameFileName = file;
	_savegameSaveRequest = true;
}

void GrimEngine::loadGame(const Common::String &file) {
	_savegameFileName = file;
	_savegameLoadRequest = true;
}

void GrimEngine::savegameRestore() {
	printf("GrimEngine::savegameRestore() started.\n");
	_savegameLoadRequest = false;
	Common::String filename;
	if (_savegameFileName.size() == 0) {
		filename = "grim.sav";
	} else {
		filename = _savegameFileName;
	}
	_savedState = SaveGame::openForLoading(filename);
	if (!_savedState || _savedState->saveVersion() != SaveGame::SAVEGAME_VERSION)
		return;
	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_movie->stop();
	g_imuse->pause(true);
	g_movie->pause(true);

	_selectedActor = NULL;
	_talkingActor = NULL;
	delete _currScene;
	_currScene = NULL;

	PoolColor::getPool()->restoreObjects(_savedState);
	Bitmap::getPool()->restoreObjects(_savedState);
	Font::getPool()->restoreObjects(_savedState);
	ObjectState::getPool()->restoreObjects(_savedState);
	Scene::getPool()->restoreObjects(_savedState);
	TextObject::getPool()->restoreObjects(_savedState);
	PrimitiveObject::getPool()->restoreObjects(_savedState);
	Actor::getPool()->restoreObjects(_savedState);
	restoreGRIM();

	g_driver->restoreState(_savedState);
	g_imuse->restoreState(_savedState);
	g_movie->restoreState(_savedState);
	_iris->restoreState(_savedState);
	lua_Restore(_savedState);

	delete _savedState;

	// Apply the patch, only if it wasn't applied already.
	if (lua_isnil(lua_getglobal("  service_release.lua"))) {
		if (bundle_dofile("patch05.bin") == 2)
			single_dofile("patch05.bin");
	}
	const char *devMode = g_registry->get("good_times", "");
	lua_beginblock();
	// Set the developerMode, since the save contains the value of
	// the installation it was made with.
	lua_pushobject(lua_getglobal("developerMode"));
	if (devMode[0] == 0)
		lua_pushnil();
	else
		lua_pushstring(devMode);
	lua_setglobal("developerMode");
	lua_endblock();

	g_imuse->pause(false);
	g_movie->pause(false);
	printf("GrimEngine::savegameRestore() finished.\n");

	_shortFrame = true;
	clearEventQueue();
}

void GrimEngine::restoreGRIM() {
	_savedState->beginSection('GRIM');

	// Actor stuff
	int32 id = _savedState->readLEUint32();
	if (id != 0) {
		_selectedActor = Actor::getPool()->getObject(id);
	}
	_talkingActor = Actor::getPool()->getObject(_savedState->readLEUint32());

	//TextObject stuff
	_sayLineDefaults.setDisabled(_savedState->readLESint32());
	_sayLineDefaults.setFGColor(PoolColor::getPool()->getObject(_savedState->readLEUint32()));
	_sayLineDefaults.setFont(Font::getPool()->getObject(_savedState->readLEUint32()));
	_sayLineDefaults.setHeight(_savedState->readLESint32());
	_sayLineDefaults.setJustify(_savedState->readLESint32());
	_sayLineDefaults.setWidth(_savedState->readLESint32());
	_sayLineDefaults.setX(_savedState->readLESint32());
	_sayLineDefaults.setY(_savedState->readLESint32());
	_sayLineDefaults.setDuration(_savedState->readLESint32());

	// Scene stuff
	_currScene = Scene::getPool()->getObject(_savedState->readLEUint32());

	_savedState->endSection();
}

void GrimEngine::storeSaveGameImage(SaveGame *state) {
	int width = 250, height = 188;
	Bitmap *screenshot;

	printf("GrimEngine::StoreSaveGameImage() started.\n");

	int mode = g_grim->getMode();
	g_grim->setMode(_previousMode);
	g_grim->updateDisplayScene();
	g_driver->storeDisplay();
	screenshot = g_driver->getScreenshot(width, height);
	g_grim->setMode(mode);
	state->beginSection('SIMG');
	if (screenshot) {
		int size = screenshot->getWidth() * screenshot->getHeight();
		screenshot->setNumber(0);
		uint16 *data = (uint16 *)screenshot->getData();
		for (int l = 0; l < size; l++) {
			state->writeLEUint16(data[l]);
		}
	} else {
		error("Unable to store screenshot");
	}
	state->endSection();
	delete screenshot;
	printf("GrimEngine::StoreSaveGameImage() finished.\n");
}

void GrimEngine::savegameSave() {
	printf("GrimEngine::savegameSave() started.\n");
	_savegameSaveRequest = false;
	char filename[200];
	if (_savegameFileName.size() == 0) {
		strcpy(filename, "grim.sav");
	} else {
		strcpy(filename, _savegameFileName.c_str());
	}
	_savedState = SaveGame::openForSaving(filename);
	if (!_savedState) {
		//TODO: Translate this!
		GUI::displayErrorDialog("Error: the game could not be saved.");
		return;
	}

	storeSaveGameImage(_savedState);

	g_imuse->pause(true);
	g_movie->pause(true);

	savegameCallback();

	PoolColor::getPool()->saveObjects(_savedState);
	Bitmap::getPool()->saveObjects(_savedState);
	Font::getPool()->saveObjects(_savedState);
	ObjectState::getPool()->saveObjects(_savedState);
	Scene::getPool()->saveObjects(_savedState);
	TextObject::getPool()->saveObjects(_savedState);
	PrimitiveObject::getPool()->saveObjects(_savedState);
	Actor::getPool()->saveObjects(_savedState);
	saveGRIM();

	g_driver->saveState(_savedState);
	g_imuse->saveState(_savedState);
	g_movie->saveState(_savedState);
	_iris->saveState(_savedState);
	lua_Save(_savedState);

	delete _savedState;

	g_imuse->pause(false);
	g_movie->pause(false);
	printf("GrimEngine::savegameSave() finished.\n");

	_shortFrame = true;
	clearEventQueue();
}

void GrimEngine::saveGRIM() {
	_savedState->beginSection('GRIM');

	//Actor stuff
	if (_selectedActor) {
		_savedState->writeLEUint32(_selectedActor->getId());
	} else {
		_savedState->writeLEUint32(0);
	}
	if (_talkingActor) {
		_savedState->writeLEUint32(_talkingActor->getId());
	} else {
		_savedState->writeLEUint32(0);
	}

	//TextObject stuff
	_savedState->writeLESint32(_sayLineDefaults.getDisabled());
	_savedState->writeLEUint32(_sayLineDefaults.getFGColor()->getId());
	_savedState->writeLEUint32(_sayLineDefaults.getFont()->getId());
	_savedState->writeLESint32(_sayLineDefaults.getHeight());
	_savedState->writeLESint32(_sayLineDefaults.getJustify());
	_savedState->writeLESint32(_sayLineDefaults.getWidth());
	_savedState->writeLESint32(_sayLineDefaults.getX());
	_savedState->writeLESint32(_sayLineDefaults.getY());
	_savedState->writeLESint32(_sayLineDefaults.getDuration());

	//Scene stuff
	_savedState->writeLEUint32(_currScene->getId());

	_savedState->endSection();
}

void GrimEngine::savegameCallback() {
	lua_Object funcParam1;

	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("saveGameCallback");
	lua_Object funcParam2 = lua_gettable();

	if (lua_istable(funcParam2)) {
		lua_pushobject(funcParam2);
		lua_pushstring("saveGameCallback");
		funcParam1 = lua_gettable();
		if (lua_isfunction(funcParam1)) {
			lua_pushobject(funcParam2);
			lua_callfunction(funcParam1);
		} else {
			error("GrimEngine::savegameCallback: handler is not a function");
		}
	} else if (lua_isfunction(funcParam2)) {
		funcParam1 = funcParam2;
		lua_callfunction(funcParam1);
	} else if (!lua_isnil(funcParam2)) {
		error("GrimEngine::savegameCallback: invalid handler");
	}
	lua_endblock();
}

Scene *GrimEngine::findScene(const Common::String &name) {
	// Find scene object
	for (Scene::Pool::Iterator i = Scene::getPool()->getBegin(); i != Scene::getPool()->getEnd(); ++i) {
		if (i->_value->getName() == name)
			return i->_value;
	}
	return NULL;
}

void GrimEngine::setSceneLock(const char *name, bool lockStatus) {
	Scene *scene = findScene(name);

	if (!scene) {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Scene object '%s' not found in list", name);
		return;
	}
	// Change the locking status
	scene->_locked = lockStatus;
}

Scene *GrimEngine::loadScene(const Common::String &name) {
	Scene *s = findScene(name);

	if (!s) {
		Common::String filename(name);
		// EMI-scripts refer to their .setb files as .set
		if (g_grim->getGameType() == GType_MONKEY4) {
			filename += "b";
		}
		Block *b = g_resourceloader->getFileBlock(filename);
		if (!b)
			warning("Could not find scene file %s", name.c_str());
		s = new Scene(name, b->getData(), b->getLen());
		delete b;
	}

	return s;
}

void GrimEngine::setScene(const char *name) {
	setScene(loadScene(name));
}

void GrimEngine::setScene(Scene *scene) {
	if (scene == _currScene)
		return;

	// Stop the actors. This fixes bug #289 (https://github.com/residual/residual/issues/289)
	// and it makes sense too, since when changing set the directions
	// and coords change too.
	for (Actor::Pool::Iterator i = Actor::getPool()->getBegin(); i != Actor::getPool()->getEnd(); ++i) {
		Actor *a = i->_value;
		a->stopWalking();
	}

	Scene *lastScene = _currScene;
	_currScene = scene;
	_currScene->setSoundParameters(20, 127);
	_currScene->setLightsDirty();
	// should delete the old scene after setting the new one
	if (lastScene && !lastScene->_locked) {
		delete lastScene;
	}
	_shortFrame = true;
}

void GrimEngine::makeCurrentSetup(int num) {
	int prevSetup = g_grim->getCurrScene()->getSetup();
	if (prevSetup != num) {
		getCurrScene()->setSetup(num);
		getCurrScene()->setSoundParameters(20, 127);
		cameraChangeHandle(prevSetup, num);
		// here should be set sound position
	}
}

void GrimEngine::setTextSpeed(int speed) {
	if (speed < 1)
		_textSpeed = 1;
	if (speed > 10)
		_textSpeed = 10;
	_textSpeed = speed;
}

float GrimEngine::getControlAxis(int num) {
	return 0;
}

bool GrimEngine::getControlState(int num) {
	return _controlsState[num];
}

float GrimEngine::getPerSecond(float rate) const {
	return rate * _frameTime / 1000;
}

Actor *GrimEngine::getTalkingActor() const {
	return _talkingActor;
}

void GrimEngine::setTalkingActor(Actor *a) {
	_talkingActor = a;
}

const Common::String &GrimEngine::getSceneName() const {
	return _currScene->getName();
}

void GrimEngine::clearEventQueue() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
	}

	for (int i = 0; i < KEYCODE_EXTRA_LAST; ++i) {
		_controlsState[i] = false;
	}
}

} // end of namespace Grim
