#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include "serial.h"
#include "config.h"
#include "device.h"

/*
 * chip_erase()
 *
 * czyści zawartość układu. zwraca -1 w przypadku powodzenia lub offset
 * pierwszej komórki, która nie jest czysta.
 */
int chip_erase()
{
	if (!chip)
		return -1;

	serial_write_ack(2, 0x44, chip->type);
	serial_write_ack(2, 0x56, chip->vpp);
	serial_write_ack(2, 0x47, 0x30);
	serial_write_ack(4, 0x41, 0x00, 0x00, 0x00);
	serial_write_ack(1, 0x4a); 

	if (serial_read() != 0x4a)
		serial_error("Invalid response");

	return chip_blank_check(0, -1);
}

/*
 * chip_blank_check()
 *
 * sprawdza, czy układ jest czysty. @offset określa pierwszą sprawdzaną
 * komórkę, @size oznacza ilość komórek do sprawdzenia. jeśli @size będzie
 * równe -1, sprawdzany jest cały układ. zwraca @size, jeśli układ jest
 * czysty lub offset pierwszej komórki, która nie jest czysta.
 */
int chip_blank_check(int offset, int size)
{
	int result, __size;

	if (offset < 0 || offset >= chip->size)
		offset = 0;

	if (size == -1)
		__size = chip->size - offset;
	else
		__size = size;

	serial_write_ack(2, 0x44, chip->type);
	serial_write_ack(2, 0x56, chip->vcc);
	serial_write_ack(2, 0x47, 0x30);
	serial_write_ack(4, 0x41, 0x00, (offset >> 8), (offset & 255));
	if (chip->bits == 16)
		__size /= 2;
	serial_write_ack(4, 0x43, 0x00, (__size >> 8), (__size & 255));
	if (chip->bits == 16)
		__size *= 2;

	result = serial_read_offset();

	serial_write_ack(1, 0x48);

	if (size == -1 && result == __size)
		result = -1;

	return result;
}

/*
 * chip_read()
 *
 * czyta @size bajtów począwszy od @offset do bufora @buffer. w przypadku
 * błędu transmisji wychodzi z programu.
 */
void chip_read(int offset, int size, unsigned char *buffer)
{
	serial_write_ack(2, 0x44, chip->type);
	serial_write_ack(2, 0x56, chip->vcc);
	serial_write_ack(2, 0x47, 0x30);
	serial_write_ack(4, 0x41, 0x00, (offset >> 8), (offset & 255));

	while (size > 0) {
		int i, chunk = (size > 0x80) ? 0x80 : size;

		if (chip->bits == 16)
			size /= 2;
		serial_write_ack(4, 0x42, 0x00, (chunk >> 8), (chunk & 255));
		if (chip->bits == 16)
			size *= 2;

		for (i = 0; i < chunk; i++)
			buffer[i] = serial_read();

		buffer += chunk;
		size -= chunk;
	}

	serial_write_ack(1, 0x48);
}

/*
 * chip_write()
 *
 * zapisuje @size bajtów układu począwszy od @offset z bufora @buffer.
 * w przypadku błędu zwraca offset komórki, która się nie zgadza, lub
 * @size, jeśli wszystko jest w porządku.
 */
int chip_write(int offset, int size, unsigned char vpp, unsigned char *buffer)
{
	int count = 0, ssize = size;

	serial_write_ack(2, 0x44, chip->type);
	serial_write_ack(2, 0x56, vpp);
	serial_write_ack(2, 0x47, 0x30);
	serial_write_ack(4, 0x41, ((offset >> 16) & 255), ((offset >> 8) & 255), (offset & 255));
	if (chip->bits == 16)
		size /= 2;
	serial_write_ack(4, 0x50, ((size >> 16) & 255), ((size >> 8) & 255), (size & 255));
	if (chip->bits == 16)
		size *= 16;

	offset = 0;

	while (size > 0) {
		int i, chunk = (size > ((chip->bits == 16) ? 0x40 : 0x20)) ? ((chip->bits == 16) ? 0x40 : 0x20) : size;

		for (i = 0; i < chunk; i++, count++)
			serial_write_nak(1, buffer[i]);

		if (chip->delay)
#ifndef WIN32
			usleep(chip->delay * 1000);
#else
			Sleep(chip->delay);
#endif

		for (i = 0; i < chunk; i++, offset++) {
			if (serial_read() != 0) {
				serial_write_nak(1, 0x48);
				return offset;
			}
		}

		buffer += chunk;
		size -= chunk;
		
		printf("\r%d%%", 100 * count / ssize);
		fflush(stdout);
	}

	serial_write_ack(1, 0x48);

	return offset;
}

/*
 * chip_fuse()
 *
 * zapisuje bity blokady lub konfiguracji. w przypadku błędu wychodzi
 * z programu.
 */
void chip_fuse(unsigned char value)
{
	serial_write_ack(2, 0x44, chip->type);
	serial_write_ack(2, 0x56, chip->vpp);
	serial_write_ack(2, 0x47, 0x30);
	serial_write_ack(4, 0x41, 0x00, 0x00, 0x00);
	serial_write_ack(2, 0x4b, 0x30 | value);

	if (serial_read() != 0x4b)
		serial_error("Invalid response");

	if (serial_read() != (0x30 | value))
		serial_error("Invalid response");

	serial_write_ack(1, 0x48);
}

/*
 * firmware_version()
 *
 * orczytuje wersję firmware programtora
 */
void firmware_version(char* fw)
{
	serial_write_ack(1, 0x5a);

	serial_read_fw_version(fw);
}

