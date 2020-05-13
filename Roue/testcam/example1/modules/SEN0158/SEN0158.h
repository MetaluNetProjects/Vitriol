/*********************************************************************
 *
 *                wiimote tracking camera SEN0158 for Fraise
 *   see https://wiki.dfrobot.com/Positioning_ir_camera
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  may 2020     Original.
 ********************************************************************/

/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/
#ifndef _SEN0158_H_
#define _SEN0158_H_

#define SEN0158_NB_CANDIDATES 10
#define SEN0158_NB_WINNERS 4
#define SEN0158_DISTANCE_THRES 150
//#define SEN0158_SCORE_THRES 20

typedef struct {
	int x, y;
	byte score;
} SEN0158point;

typedef struct {
	byte address;
	SEN0158point inpoints[4];
	SEN0158point points[SEN0158_NB_CANDIDATES];
	char winners[SEN0158_NB_WINNERS];
	byte status;
} SEN0158;

void SEN0158Init(SEN0158 *dev);
void SEN0158Service(SEN0158 *dev);
void SEN0158Send(SEN0158 *dev, byte prefix);
void SEN01585Set(SEN0158 *dev, byte reg, byte value);
void SEN01585SetDistanceThreshold(unsigned int thres);


#endif	/* _SEN0158_H_ */
