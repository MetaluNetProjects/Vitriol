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
#include <fruit.h>
#include <i2c_master.h>
#include <SEN0158.h>

static byte address;
static byte i, j, k;
static byte b1, b2, b3;
static int x, y;
static byte size;
static unsigned int distance, best_distance;
static byte best_index;
static byte sorted[SEN0158_NB_CANDIDATES];
static unsigned int distance_threshold = SEN0158_DISTANCE_THRES;

static void delay(word millisecs)
{
	t_delay del;
	delayStart(del, millisecs * 1000);
	while(!delayFinished(del)){}
}

static void writeTo(byte address, byte reg, byte value)
{
	i2cm_begin(address,0);
	i2cm_writechar(reg);
	i2cm_writechar(value);
	i2cm_stop();
}

void SEN0158Set(SEN0158 *dev, byte reg, byte value)
{
	writeTo(dev->address, reg, value);
}

void SEN0158Init(SEN0158 *dev)
{
	byte i;
	dev->address = 0x58;

	for(i = 0; i < SEN0158_NB_CANDIDATES; i++) {
		dev->points[i].x = dev->points[i].y = 512;
		dev->points[i].score = 0;
		sorted[i] = i;
	}

	for(i = 0; i < SEN0158_NB_WINNERS; i++) {
		dev->winners[i] = (char)-1;
	}

	//SEN0158 TURN ON
	writeTo(dev->address, 0x30, 0x01); delay(10);
	writeTo(dev->address, 0x30, 0x08); delay(10);
#if 0 // default:
	writeTo(dev->address, 0x06, 0x90); delay(10);
	writeTo(dev->address, 0x08, 0xC0); delay(10);
	writeTo(dev->address, 0x1A, 0x40); delay(10);
#else // max sensitivity:
	writeTo(dev->address, 0x00, 0x00); delay(10);
	writeTo(dev->address, 0x01, 0x00); delay(10);
	writeTo(dev->address, 0x02, 0x00); delay(10);
	writeTo(dev->address, 0x03, 0x00); delay(10);
	writeTo(dev->address, 0x04, 0x00); delay(10);
	writeTo(dev->address, 0x05, 0x00); delay(10);
	writeTo(dev->address, 0x06, 0xFF); delay(10);
	writeTo(dev->address, 0x07, 0x00); delay(10);
	writeTo(dev->address, 0x08, 0x0C); delay(10);
	writeTo(dev->address, 0x1A, 0x00); delay(10);
	writeTo(dev->address, 0x1B, 0x00); delay(10);
#endif

	writeTo(dev->address, 0x33, 0x33); delay(10);
	delay(100);
}

#define BEST_PRICE 5

void SEN0158Service(SEN0158 *dev)
{
	address = dev->address;
	
	i2cm_begin(address, 0);
	i2cm_writechar(0x36);
	i2cm_stop();

	i2cm_begin(address, 1);
	
	dev->status = i2cm_readchar(); i2cm_ack();
	
	for (i = 0; i < 4; i++) {
		b1 = i2cm_readchar(); i2cm_ack();
		b2 = i2cm_readchar(); i2cm_ack();
		b3 = i2cm_readchar(); i2cm_ack();
		dev->inpoints[i].x = x = b1 + ((b3 & 0x30) << 4);
		dev->inpoints[i].y = y = b2 + ((b3 & 0xC0) << 2);
		dev->inpoints[i].score = size = b3 & 0x0F;
		best_distance = 10000;
		best_index = 0;
		if((y != 1023) /*&& (size > 0) && (size < 15)*/) {
			for(j = 0; j < SEN0158_NB_CANDIDATES; j++) {
				distance = (unsigned int)abs(x - dev->points[j].x) + abs(y - dev->points[j].y);
				if(distance < best_distance) {
					best_distance = distance;
					best_index = j;
				}
			}
			if(best_distance <= distance_threshold) {
				// update winner position, give him the price:
				dev->points[best_index].x = x;
				dev->points[best_index].y = y;
				if(dev->points[best_index].score < (255 - BEST_PRICE))
					dev->points[best_index].score += BEST_PRICE;
			} else {
				// store position to worst scored point:
				b1 = 255;
				best_index = 0;
				for(j = 0; j < SEN0158_NB_CANDIDATES; j++) {
					if(dev->points[j].score < b1) {
						b1 = dev->points[j].score;
						best_index = j;
					}
				}
				dev->points[best_index].x = x;
				dev->points[best_index].y = y;
				dev->points[best_index].score = BEST_PRICE;
			}
		}
	}
	i2cm_stop();
	// decrement every score:
	for(j = 0; j < SEN0158_NB_CANDIDATES; j++) {
		if(dev->points[j].score > 0) dev->points[j].score--;
		else {
			dev->points[j].x = dev->points[j].x = 1023;
		}
	}
}

void SEN0158Send(SEN0158 *dev, byte prefix)
{
	byte buf[SEN0158_NB_WINNERS * 4 + 4];
	byte tmp, len = 0;
	byte index, score;
	//byte sorted[SEN0158_NB_CANDIDATES];

	// sort channels descendingly, by insertion algorithm (https://en.wikipedia.org/wiki/Insertion_sort)
	for(i = 1; i < SEN0158_NB_CANDIDATES; i++) {
		j = i;
		while((j > 0) && (dev->points[sorted[j-1]].score < dev->points[sorted[j]].score)) {
			tmp = sorted[j-1];
			sorted[j-1] = sorted[j];
			sorted[j] = tmp;
			j--;
		}
	}

	// attribute a persistant index to winners
	for(i = 0; i < SEN0158_NB_WINNERS; i++) {
		tmp = 0;
		index = sorted[i];
		for(j = 0; j < SEN0158_NB_WINNERS; j++) {
			if(index == dev->winners[j]) {
				tmp = 1;
				break;
			}
		}
		if(!tmp) { // this winner wasn't already attributed; replace the worst attributed one
			tmp = 0;
			score = 255;
			for(j = 0; j < SEN0158_NB_WINNERS; j++) {
				if(dev->points[dev->winners[j]].score < score) {
					score = dev->points[dev->winners[j]].score;
					tmp = j;
				}
			}
			dev->winners[tmp] = index;
		}
	}
	
	buf[len++] = 'B';
	buf[len++] = prefix;
	
	//for(i = 1; i < sizeof(SEN0158); i++) buf[len++] = ((byte*)dev)[i];
	for (i = 0; i < SEN0158_NB_WINNERS; i++) {
		/*if(dev->points[sorted[i]].score < SEN0158_SCORE_THRES) {
			buf[len++] = 255;
			buf[len++] = 15;
			buf[len++] = 255;
			buf[len++] = 15;
		} else*/ {
			buf[len++] = dev->points[dev->winners[i]].x & 0xFF;
			buf[len++] = dev->points[dev->winners[i]].x >> 8;
			buf[len++] = dev->points[dev->winners[i]].y & 0xFF;
			buf[len++] = dev->points[dev->winners[i]].y >> 8;
			buf[len++] = dev->points[dev->winners[i]].score;
		}
	}
	buf[len++] = '\n';
	fraiseSend(buf,len);

	return;
	/*len = 0;
	buf[len++] = 'B';
	buf[len++] = prefix + 1;
	for(i = 0; i < 4; i++) {
		buf[len++] = dev->inpoints[i].x & 0xFF;
		buf[len++] = dev->inpoints[i].x >> 8;
		buf[len++] = dev->inpoints[i].y & 0xFF;
		buf[len++] = dev->inpoints[i].y >> 8;
		buf[len++] = dev->inpoints[i].score;
	}
	buf[len++] = '\n';
	fraiseSend(buf,len);*/
}

void SEN01585SetDistanceThreshold(unsigned int thres)
{
	distance_threshold = thres;
}

void SEN0158SetupFromFraise(SEN0158 *dev)
{
	//SEN0158 TURN ON
	writeTo(dev->address, 0x30, 0x01); delay(10);
	writeTo(dev->address, 0x30, 0x08); delay(10);
	
	writeTo(dev->address, 0x00, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x01, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x02, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x03, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x04, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x05, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x06, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x07, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x08, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x1A, fraiseGetChar()); delay(10);
	writeTo(dev->address, 0x1B, fraiseGetChar()); delay(10);
	
	writeTo(dev->address, 0x33, 0x33); delay(10);
}


