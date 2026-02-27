#ifndef __CHIPS_H
#define __CHIPS_H

#define VCC5   0x00
#define VPP5   0x20
#define VPP12  0x21
#define VPP21  0x22
#define VPP25  0x23
#define VCC5E  0x40
#define VPP5E  0x40
#define VCC5M  0x60
#define VCC12M 0x61
#define VPP5M  0x60
#define VPP12M 0x61

struct chip_s {
	char *name;
	unsigned char type;
	unsigned long size;
	unsigned char bits;
	unsigned char vcc;
	unsigned char vpp;
	unsigned long delay;
	unsigned char algmax;
};

extern struct chip_s chips[];

extern struct chip_s *chip;

#endif
