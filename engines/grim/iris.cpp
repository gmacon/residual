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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/iris.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/savegame.h"
#include "engines/grim/grim.h"

namespace Grim {

Iris::Iris() :
	_playing(false), _direction(Open) {

}

Iris::~Iris() {

}

void Iris::play(Iris::Direction dir, int x, int y, int lenght) {
	_playing = true;
	_direction = dir;
	_targetX = x;
	_targetY = y;
	_lenght = lenght;
	_currTime = 0;
}

void Iris::draw() {
	if (!_playing) {
		if (_direction == Close && g_grim->getMode() != ENGINE_MODE_SMUSH) {
			g_driver->irisAroundRegion(320, 240, 320, 240);
		}
		return;
	}

	g_driver->irisAroundRegion(_x1, _y1, _x2, _y2);
}

void Iris::update(int frameTime) {
	if (!_playing) {
		return;
	}

	_currTime += frameTime;
	if (_currTime >= _lenght) {
		_playing = false;
		return;
	}

	float factor = (float)_currTime / (float)_lenght;
	if (_direction == Open) {
		factor = 1 - factor;
	}

	_y1 = (int)(_targetY * factor);
	_x1 = (int)(_targetX * factor);
	_y2 = (int)(480 - (480 - _targetY) * factor);
	_x2 = (int)(640 - (640 - _targetX) * factor);
}

void Iris::saveState(SaveGame *state) const {
	state->beginSection('IRIS');

	state->writeLEBool(_playing);
	state->writeLEUint32((uint32)_direction);
	state->writeLEUint32(_x1);
	state->writeLEUint32(_y1);
// 	state->writeLEUint32(_x2);
// 	state->writeLEUint32(_y2);
	state->writeLEUint32(_lenght);
	state->writeLEUint32(_currTime);

	state->endSection();
}

void Iris::restoreState(SaveGame *state) {
	state->beginSection('IRIS');

	_playing = state->readLEBool();
	_direction = (Direction)state->readLEUint32();
	_x1 = state->readLEUint32();
	_y1 = state->readLEUint32();
// 	_x2 = state->readLEUint32();
// 	_y2 = state->readLEUint32();
	_lenght = state->readLEUint32();
	_currTime = state->readLEUint32();

	state->endSection();
}

}
