#ifndef __CHIPS_H
#define __CHIPS_H

#define VCC5 0x00
#define VCC12 0x61
#define VPP5 0x20
#define VPP12 0x21
#define VPP21 0x22
#define VPP25 0x23

struct chip {
	char *name;
	unsigned char type;
	unsigned long size;
	unsigned char bits;
	unsigned char vcc;
	unsigned char vpp;
	unsigned long delay;
};

extern struct chip chips[];

struct chip *chip;

#endif
