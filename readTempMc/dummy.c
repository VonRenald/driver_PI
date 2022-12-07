#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "ioctl.h"

// #define MY_IOCTL_IN _IOC(_IOC_WRITE, 'k', 1, sizeof(my_ioctl_data))
#define MY_MAJOR       42
#define MY_MAX_MINORS  1

/* Sélection du numéro du bus avec le paramètre `bus_pin'. */
int bus_pin = 23;


MODULE_DESCRIPTION("UwUTemp");
MODULE_AUTHOR("Me");
MODULE_LICENSE("GPL");


static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg);
static int my_open(struct inode *inode, struct file *file);
static int my_read(struct file *file, char __user *user_vuffer, size_t size, loff_t *offset);
static int my_release(struct inode *, struct file *);
static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg);
static int my_read_temp(struct file *file, char __user *user_vuffer, size_t size, loff_t *offset);

inline void onewire_high(void);
inline void onewire_low(void);
int onewire_reset(void);
inline int onewire_read(void);
inline void onewire_write_one(void);
u8 onewire_read_byte(void);
inline void onewire_write_zero(void);
u8 onewire_crc8(const u8 *data, size_t len);
void onewire_write_byte(u8 b);



enum read_type { STRING, TEMP};

struct sav_rom_branch {
    u64 rom;
    unsigned char len;
};

struct my_device_data {
    struct cdev cdev;
    int nb_capteur_connect;
    u64 capteur[8];
};

int serach_capteur(struct my_device_data* my_data);
void print_rom(u64 id);
void match_rom(u64 id);
void convT(void);
int readTemp(char* str, int len_str);

struct my_device_data devs[MY_MAX_MINORS];

const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    // .write = my_write,
    .release = my_release,
    .unlocked_ioctl = my_ioctl

};

int init_module(void)
{
    // pr_warn("--------\n");
    int i, err;

    err = register_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS,
                                 "my_device_driver");
    if (err != 0) {
        // pr_warn("driver init fail\n");
        return err;
    }

    for(i = 0; i < MY_MAX_MINORS; i++) {
        /* initialize devs[i] fields */
        cdev_init(&devs[i].cdev, &my_fops);
        cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
    }
    // pr_warn("driver init succes\n");
    return 0;
}

void cleanup_module(void)
{
    int i;

    for(i = 0; i < MY_MAX_MINORS; i++) {
        /* release devs[i] fields */
        cdev_del(&devs[i].cdev);
    }
    unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS);
    // pr_warn("driver close\n");
}

/* my_open, my_read etc. to implement */
static int my_read(struct file *file, char __user *user_vuffer, size_t size, loff_t *offset)
{
    struct my_device_data *my_data;

    my_data = (struct my_device_data *) file->private_data;

    pr_warn("---\n");

    char str[255];
    char final_str[255];
    int i,len_str;
    len_str = 0;
    for(i=0;i<my_data->nb_capteur_connect;i++)
    {
         match_rom(my_data->capteur[i]);
        convT();   
        match_rom(my_data->capteur[i]);
        onewire_write_byte(0xBE);
        len_str = readTemp(str, 255);
        if(i == 0)
            sprintf(final_str,"C %d : %s",i,str);
        else
            sprintf(final_str,"%s\nC %d : %s",final_str,i,str);
    }



    i = 0;
    while (final_str[i] !='\0' && i < 255 && i<size) {i++;}
    pr_warn("%s\n",final_str);
    pr_warn("i - %d\n",i);
    if(i>size){return 0;}

    copy_to_user(user_vuffer,final_str,i);
    

    return (i);
}

static int my_open(struct inode *inode, struct file *file)
{
    struct my_device_data *my_data = container_of(inode->i_cdev, struct my_device_data, cdev);

    /* validate access to device */
    file->private_data = my_data;

    // print_rom(id);
    pr_warn(">\n");
    serach_capteur(my_data);
    int i = 0;
    while(i<my_data->nb_capteur_connect) {
        print_rom(my_data->capteur[i]);
        i++;
    }
    pr_warn("<\n");
  

    
    return 0;
}
static int my_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
    struct my_device_data *my_data = (struct my_device_data*) file->private_data;
    struct my_ioctl_data mid;


    switch(cmd) {
    case MY_IOCTL_IN:

        break;
    default:
        return -ENOTTY;
    }

    return 0;
}

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
	} while ((!was_pulled_down || (was_pulled_down && !val)) && retries < 50);

	if (retries == 50) {
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
//retourn le nombre de cateur trouvé
//ecrit les capteur dans my_data->capteur
int serach_capteur(struct my_device_data* my_data)
{
    
    int ret, bit1, bit2, i, j; 
    u64 id, mask;
    struct sav_rom_branch step_branch[32];
    unsigned char len_step_branch;
    bool isIn;

    len_step_branch = 0;

    i=0;
    mask = 1;
    ret = onewire_reset();
    onewire_write_byte(0xF0);//serach command
    id = 0;
    // ETAPE 1  parcour une premiere fois une addresse pour trouver un capteur et les point de divergences
    while (i<64) // parcour bit
    {
        bit1 = onewire_read(); // bit
        bit2 = onewire_read(); // complement
        

        if(bit1 == 0 && bit2 == 0 ) // 2 possibility
        {
            step_branch[len_step_branch].rom = id;
            step_branch[len_step_branch].len = i;
            len_step_branch++;
            id = id << 1;
            onewire_write_zero();
        }
        else 
        {
            id = id << 1;
            if(bit1 == 1) {
                onewire_write_one();
                id += mask;
            }
            else {
                onewire_write_zero();
            }
        }
        i++;
    }

    // verifie que le capteur n'est pas deja dans la liste des capteur connu 
    i=0;
    isIn = false;
    while(i<my_data->nb_capteur_connect){
        isIn = isIn || (id == my_data->capteur[i]);
        i++;
    }
    if (!isIn){
        my_data->capteur[my_data->nb_capteur_connect] = id;
        my_data->nb_capteur_connect++;    
    }
    
    // ETAPE 2  temps que la liste des points de divergence n'est pas vide, on parcourt les possibilité afin de decouvrire 
    // les autres capteur et point de divergence
    i = 0;
    while(i<len_step_branch)
    {
        id = step_branch[i].rom;
        ret = onewire_reset();
        onewire_write_byte(0xF0);
        j = 0;
        while (j != step_branch[i].len)
        {   
            
            bit1 = onewire_read();
            bit2 = onewire_read();
            // pr_warn("1 : %d -> %d,%d\t%llu\n",j, bit1, bit2, ((id >> (step_branch[i].len-j-1)) & mask));
            if(((id >> (step_branch[i].len-j-1)) & mask) == 1)
            {onewire_write_one();}
            else{onewire_write_zero();}
            j++;
        }
        bit1 = onewire_read();
        bit2 = onewire_read();
        // pr_warn("2 : %d -> %d,%d\n",j, bit1,bit2);
        onewire_write_one();
        j++;
        
        id <<=1;
        id += mask;
        while(j<64)
        {
            bit1 = onewire_read();
            bit2 = onewire_read();
            // pr_warn("3 : %d -> %d,%d\n",j, bit1,bit2);

            if(bit1 == 0 && bit2 == 0 )
            {
                step_branch[len_step_branch].rom = id;
                step_branch[len_step_branch].len = j;
                len_step_branch++;
                id = id << 1;
                onewire_write_zero();
            }
            else 
            {
                
                id = id << 1;
                if(bit1 == 1) {
                    onewire_write_one();
                    id += mask;
                    // pr_warn("ici\n");
                }
                else {
                    onewire_write_zero();
                    // pr_warn("la\n");
                }
            }
            j++;
        }
        i++;
        // print_rom(id);
        j=0;
        isIn = false;
        while(j<my_data->nb_capteur_connect){
            isIn = isIn || (id == my_data->capteur[j]);
            j++;
        }
        if (!isIn){
            my_data->capteur[my_data->nb_capteur_connect] = id;
            my_data->nb_capteur_connect++;    
        }
    }
    return 0;
}

void print_rom(u64 id)
{
    int i=0;
    u64 mask = 1;
    char str[65];
    while(i<64)
    {
        str[63-i] = '0' + (char) (id>>i & 1);
        //pr_warn("%llu\n", (id>>i & 1) );
        i++;
    }
    str[64] = '\0';
    pr_warn("ROM : %s\n",str);
    return;
}

void match_rom(u64 id)
{

    onewire_reset();
    onewire_write_byte(0x55);

    u64 mask;
    mask = 1;
    mask = mask << 63;
    // pr_warn(">%llu<\n",mask);
    int i = 1;
	for (; mask; mask >>= 1) {
		if (id & mask){
			onewire_write_one();
            // pr_warn("%d 1\n",i);
            }
		else{
			onewire_write_zero();
            // pr_warn("%d 0\n",i);
            }
        i++;
	}

    // onewire_reset();
    // onewire_write_byte(0x44);

    // int test2 = onewire_read_byte();
    // while( test2 == 0) 
    // { 
    //     //pr_warn("sleep %d\n",test2); 
    //     fsleep (10);
    //     test2 = onewire_read_byte();
    // }



    // //LSB 128 MSB 1    128 64 32 16 8 4 2 1
    // // 2^3
    // char lsb = onewire_read_byte(); 
    // // pr_warn("LSB : %d\n",(int) lsb);
    // // pr_warn("LSB entier : %d\n",(int) lsb >> 4);
    // char msb = onewire_read_byte();

    // pr_warn("%d %d\n",lsb,msb);

    // char signe = msb & 0b10000000;
    // msb =  msb & 0b00000111;

    // int floatingPart = (lsb & 0b00001111);
    // int tempF = floatingPart * 625;
    
    // lsb = lsb >> 4;
    // msb = msb << 4;
    // int tempI = (signe == 1)? -(lsb+msb):lsb+msb;

    // pr_warn("temp : %d %d %d\n",tempI, tempF, floatingPart);

    // char str[255];
    // if(floatingPart == 1) {sprintf(str,"%d.0%d", tempI, tempF);}
    // else {sprintf(str,"%d.%d", tempI, tempF);}
    // pr_warn("str : %s\n", str);
    // int i = 0;
    // while (str[i] !='\0' && i < 255 ) {i++;}

}

void convT(void)
{
    onewire_write_byte(0x44);
    onewire_high();
	usleep_range(60, 65);

    int test2 = onewire_read();
    // pr_warn("convt : %d\n",test2);
    while( test2 == 0) 
    { 
        //pr_warn("sleep %d\n",test2); 
        usleep_range(200, 205);
        test2 = onewire_read();
        // pr_warn("convt : %d\n",test2);
    }
}

int readTemp(char* str, int len_str)
{
    char lsb = onewire_read_byte(); 
    // pr_warn("LSB : %d\n",(int) lsb);
    // pr_warn("LSB entier : %d\n",(int) lsb >> 4);
    char msb = onewire_read_byte();

    pr_warn("%d %d\n",lsb,msb);

    char signe = msb & 0b10000000;
    msb =  msb & 0b00000111;

    int floatingPart = (lsb & 0b00001111);
    int tempF = floatingPart * 625;
    
    lsb = lsb >> 4;
    msb = msb << 4;
    int tempI = (signe == 1)? -(lsb+msb):lsb+msb;

    pr_warn("temp : %d %d %d\n",tempI, tempF, floatingPart);

    // char str[255];
    if(floatingPart == 1) {sprintf(str,"%d.0%d", tempI, tempF);}
    else {sprintf(str,"%d.%d", tempI, tempF);}
    pr_warn("str : %s\n", str);
    int i = 0;
    while (str[i] !='\0' && i < len_str ) {i++;}
    return i;
}
