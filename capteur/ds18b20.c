/* Driver DS18B20 simple
 *
 * Utilisation : après insertion, ce module utilise device_create() pour
 * créer un périphérique automatiquement (/dev/temp0).
 *
 * Pin par défaut : 17, modifiable avec le paramètre `bus_pin'.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

/* Sélection du numéro du bus avec le paramètre `bus_pin'. */
int bus_pin = 23;

/* Fonctions onewire */
inline void onewire_high(void)
{
	/* Bus à l'état haut réalisé en plaçant le pin en mode entrée. */
	gpio_direction_input(bus_pin);
}

inline void onewire_low(void)
{
	/* Bus à l'état bas réalisé en plaçant le pin en mode sortie, valeur
	   0. */
	gpio_direction_output(bus_pin, 0);
}

int onewire_reset(void)
{
	int val, was_pulled_down = 0, retries = 0;

	onewire_low();
	usleep_range(480, 500);
	onewire_high();

	usleep_range(15, 20);

	do {
		udelay(5);
		val = gpio_get_value(bus_pin);

		if (!was_pulled_down && !val)
			was_pulled_down = 1;
		++retries;
	} while ((!was_pulled_down || (was_pulled_down && !val)) && retries < NUM_RETRIES);

	if (retries == NUM_RETRIES) {
		printk(KERN_ALERT "failed to reset onewire bus (50 retries)\n");
		return -ECANCELED;
	}

	udelay(5);
	return 0;
}

inline int onewire_read(void)
{
	int res;

	/* Ouverture d'une fenêtre de lecture, cf. datasheet page 16. */
	onewire_low();
	udelay(5);
	onewire_high();

	/* 15µs après le front descendant, si le bus est à l'état bas, un 0 a
	   été écrit, sinon un 1 a été écrit. */
	udelay(10);
	res = gpio_get_value(bus_pin);
	usleep_range(50, 55);

	return res;
}

u8 onewire_read_byte(void)
{
	int i;
	u8 val = 0;

	/* Lecture d'un octet. */
	for (i = 0; i < 8; ++i)
		val |= onewire_read() << i;

	return val;
}

inline void onewire_write_zero(void)
{
	/* Écriture d'un 0 : on met le bus à l'état bas pour 60µs, cf. datasheet
	   page 15. */
	onewire_low();
	usleep_range(60, 65);
	onewire_high();
	udelay(5);
}

inline void onewire_write_one(void)
{
	/* Écriture d'un 1 : on met le bus à l'état bas pour 5µs, et on attend
	   60µs, cf. datasheet page 15. */
	onewire_low();
	udelay(5);
	onewire_high();
	usleep_range(60, 65);
}

void onewire_write_byte(u8 b)
{
	u8 mask;

	/* Écriture d'un octet. */
	for (mask = 1; mask; mask <<= 1) {
		if (b & mask)
			onewire_write_one();
		else
			onewire_write_zero();
	}
}

u8 onewire_crc8(const u8 *data, size_t len)
{
	size_t i;
	u8 shift_register = 0;  /* Shift register à 0. */

	/* Calcul d'un CRC, selon la description sur la datasheet page 9. */
	/* Pour chaque octet disponible en données… */
	for (i = 0; i < len; ++i) {
		u8 mask;

		/* Pour chaque bit de chaque octet… */
		for (mask = 1; mask; mask <<= 1) {
			/* input = (bit courant) xor shifted_register[0] */
			u8 input = (!!(data[i] & mask)) ^ (shift_register & 0x01);

			/* Shift à gauche du registre, masquage des bits 3 et
			   4 pour les remplacer. */
			u8 shifted_register = (shift_register >> 1) & ~0x0C;

			/* shifted_register[2] = input xor shift_register[3] */
			shifted_register |= ((!!(shift_register & 0x08)) ^ input) << 2;

			/* shifted_register[3] = input xor shift_register[4] */
			shifted_register |= ((!!(shift_register & 0x10)) ^ input) << 3;

			/* shifted_register[7] = input */
			shifted_register |= input << 7;
			shift_register = shifted_register;
		}
	}

	return shift_register;
}
