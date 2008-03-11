/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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
 * $URL$
 * $Id$
 *
 */

#ifndef IMUSE_H
#define IMUSE_H

#include "common/sys.h"
#include "common/platform.h"
#include "common/debug.h"

#include "engine/lua.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#include "engine/imuse/imuse_sndmgr.h"
#include "engine/imuse/imuse_mcmp_mgr.h"
#include "engine/imuse/imuse_track.h"
#include "engine/imuse/imuse_tables.h"

#define MAX_IMUSE_TRACKS 16
#define MAX_IMUSE_FADETRACKS 16

class SaveGame;

class Imuse {
private:

	int _callbackFps;

	Track *_track[MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS];

	Common::Mutex _mutex;
	ImuseSndMgr *_sound;

	bool _pause;

	int32 _attributes[185];
	int32 _curMusicState;
	int32 _curMusicSeq;

	const ImuseTable *_stateMusicTable;
	const ImuseTable *_seqMusicTable;

	int32 makeMixerFlags(int32 flags);
	static void timerHandler(void *refConf);
	void callback();
	void switchToNextRegion(Track *track);
	int allocSlot(int priority);
	void selectVolumeGroup(const char *soundName, int volGroupId);

	void fadeOutMusic(int fadeDelay);
	void fadeOutMusicAndStartNew(int fadeDelay, const char *filename, int hookId, int vol, int pan);
	Track *cloneToFadeOutTrack(Track *track, int fadeDelay);

	void playMusic(const ImuseTable *table, int atribPos, bool sequence);

	void flushTrack(Track *track);

public:
	Imuse(int fps);
	~Imuse();

	bool startSound(const char *soundName, int volGroupId, int hookId, int volume, int pan, int priority, Track *otherTrack);
	void startVoice(const char *soundName, int volume = 127, int pan = 64);
	void startMusic(const char *soundName, int hookId, int volume, int pan);
	void startMusicWithOtherPos(const char *soundName, int hookId, int volume, int pan, Track *otherTrack);
	void startSfx(const char *soundName, int priority = 127);

	void restoreState(SaveGame *savedState);
	void saveState(SaveGame *savedState);
	void resetState();

	Track *findTrack(const char *soundName);
	void setPriority(const char *soundName, int priority);
	void setVolume(const char *soundName, int volume);
	int getVolume(const char *soundName);
	void setPan(const char *soundName, int pan);
	void setFadePan(const char *soundName, int destPan, int duration);
	void setFadeVolume(const char *soundName, int destVolume, int duration);
	void setHookId(const char *soundName, int hookId);
	int getCountPlayedTracks(const char *soundName);
	void stopSound(const char *soundName);
	void stopAllSounds();
	void pause(bool pause);
	void setMusicState(int stateId);
	int setMusicSequence(int seqId);
	void refreshScripts();
	void flushTracks();
	bool isVoicePlaying();
	char *getCurMusicSoundName();
	int getCurMusicPan();
	int getCurMusicVol();
	bool getSoundStatus(const char *soundName);
	int32 getPosIn60HzTicks(const char *soundName);
};

extern Imuse *g_imuse;

#endif