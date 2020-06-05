#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define NETWORK_ID 10
void fillPipeAddress(uint8_t* address_buffer, uint8_t sender, uint8_t receiver)
{
	address_buffer[0] = (sender<<4) + receiver;
	address_buffer[1] = NETWORK_ID;
	address_buffer[2] = 0x99;
	address_buffer[3] = 0x98;
	address_buffer[4] = 0x97;
}

#define ID_MASTER	0
#define ID_CAMERA	1
#define ID_AXL_A	2
#define ID_AXL_B	3
#define ID_AXL_C	4
#define ID_AXL_D	5
#define ID_AXL_E	6

#define CMD_VBATT		0	//
#define CMD_CAMERA		1	//
#define CMD_AXL			2	//

#endif // _PROTOCOL_H_

