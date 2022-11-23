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

int serach_capteur(my_device_data* my_data);

enum read_type { STRING, TEMP};

struct sav_rom_branch {
    unsigned long long int rom;
    unsigned char len;
}

struct my_device_data {
    struct cdev cdev;
    // int size;
    // char buffer[10];
    // char* word;
    // int len_word;

    int nb_capteur_connect;
    unsigned long long int capteur[8];

    /* my data starts here */
    //...
};


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
    


    int test = onewire_reset();
    onewire_write_byte(0xCC);
    // pr_warn("->");
    // pr_warn("%d\n",test);

    onewire_write_byte(0x44);
    int test2 = onewire_read_byte();
    while( test2 == 0) 
    { 
        //pr_warn("sleep %d\n",test2); 
        fsleep (10);
        test2 = onewire_read_byte();
    }
    // pr_warn(">%d\n",test2);

    test = onewire_reset();
    onewire_write_byte(0xCC);
    // pr_warn("->");
    // pr_warn("return reset : %d\n",test);
    onewire_write_byte(0xBE);
    //LSB 128 MSB 1    128 64 32 16 8 4 2 1
    // 2^3
    char lsb = onewire_read_byte(); 
    // pr_warn("LSB : %d\n",(int) lsb);
    // pr_warn("LSB entier : %d\n",(int) lsb >> 4);
    char msb = onewire_read_byte();
    // pr_warn("MSB : %d\n",msb);

    test = onewire_reset();
    onewire_write_byte(0xCC);


    char signe = msb & 0xb10000000;
    msb =  msb & 0xb00000111;

    int floatingPart = (lsb & 0xb00001111);
    int tempF = floatingPart * 625;
    
    lsb = lsb >> 4;
    msb = msb << 4;
    int tempI = (signe == 1)? -(lsb+msb):lsb+msb;

    pr_warn("temp : %d %d %d\n",tempI, tempF, floatingPart);

    


    char str[255];
    if(floatingPart == 1) {sprintf(str,"%d.0%d", tempI, tempF);}
    else {sprintf(str,"%d.%d", tempI, tempF);}
    pr_warn("str : %s\n", str);
    int i = 0;
    while (str[i] !='\0' && i < 255 && i<size) {i++;}
    
    //if(i>size){return 0;}

    copy_to_user(user_vuffer,str,i);
    

    // int i = 0;
    // while(i<10 && str[i] != '\0')
    // {
    //     user_vuffer[i] = str[i];
    //     i++;
    // }
    // user_vuffer[i] = '\0';
    // i++;

    // user_vuffer[0] =  temp;

    return (i);
}

static int my_open(struct inode *inode, struct file *file)
{
    struct my_device_data *my_data = container_of(inode->i_cdev, struct my_device_data, cdev);

    /* validate access to device */
    file->private_data = my_data;


    /* initialize device */
    // my_data->len_word = 5;
    // my_data->word = kmalloc ((my_data->len_word)+1 , GFP_KERNEL);
    // my_data->word[0] = 'S';
    // my_data->word[1] = 'E';
    // my_data->word[2] = 'C';
    // my_data->word[3] = 'I';
    // my_data->word[4] = 'L';


    int ret; 
    ret = onewire_reset();
    onewire_write_byte(0xF0);
    int br1, br2; 
    int i;
    char id_char[65];
    char id_char2[65];
    unsigned long long int id;
    unsigned long long int ull_mask;
    ull_mask = 1; //ull_mask = ull_mask << 63;
    id = 0;
    i=0;
    while (i<64)
    {
        br1 = onewire_read(); 
        br2 = onewire_read(); 
        // pr_warn("bit read %d %d %d\n",i, br1, br2);
        
        id = id << 1;
        if(br1 == 1) {
            // pr_warn("write 1\n"); 
            onewire_write_one();
            id += ull_mask;
        }
        else {
            // pr_warn("write 0\n"); 
            onewire_write_zero();
        }
        id_char[i] = '0' + br1;
        pr_warn("%llu\n",id); 
        i++;
    }
    id_char[64] = '\0';
    pr_warn("id : %s %llu\n",id_char,id);
    
    i=0;
    ull_mask = 1;
    while (i<64)
    {
        id_char2[63-i] = '0' + (id & ull_mask);
        id = id >> 1;
        i++;
    }
    id_char2[64] = '\0';
    pr_warn("id : %s\n",id_char2);

    // br1 = onewire_read(); 
    // // onewire_low();
    // br2 = onewire_read(); 
    // pr_warn("bit read 1 %d %d\n",br1,br2);
    // // onewire_write_zero();

    // br1 = onewire_read(); 
    // br2 = onewire_read(); 
    // pr_warn("bit read 2 %d %d\n",br1,br2);

    // // onewire_write_one();

    // br1 = onewire_read(); 
    // br2 = onewire_read(); 
    // pr_warn("bit read 3 %d %d\n",br1,br2);
    

    
    return 0;
}
static int my_release(struct inode *inode, struct file *file)
{
    return 0;
}
/** write
    // static int my_write(struct file *file, const char __user *user_buffer,size_t size, loff_t * offset)
    // {
    //     struct my_device_data *my_data = (struct my_device_data *) file->private_data;
    //     ssize_t len = min(my_data->size - *offset, size);

    //     if (len <= 0)
    //         return 0;

    //     if (copy_from_user(my_data->buffer + *offset, user_buffer, len))
    //         return -EFAULT;

    //     *offset += len;
    //     return len;
    // }
**/
static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
    struct my_device_data *my_data = (struct my_device_data*) file->private_data;
    struct my_ioctl_data mid;


    switch(cmd) {
    case MY_IOCTL_IN:
        

        // int ret = copy_from_user(&mid, (struct My_ioctl_data *) arg, sizeof(struct my_ioctl_data));
        // int i=0;
        // while(i<my_data->len_word)
        // {
        //     pr_warn("%c",my_data->word[i]);  
        //     i++;
        // }

        // pr_warn("my_data->len_word %d\n",my_data->len_word);
        // my_data->len_word = mid.len_word;
        // pr_warn("my_data->len_word %d\n",my_data->len_word);
        // kfree(my_data->word);
        // my_data->word = kmalloc ((my_data->len_word)+1 , GFP_KERNEL);
        // copy_from_user(my_data->word,mid.word,my_data->len_word);
        // i=0;
        // while(i<my_data->len_word)
        // {
        //     pr_warn("%c",mid.word[i]);  
        //     i++;
        // }
        // return -EFAULT;

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
int serach_capteur(my_device_data* my_data)
{
    int ret, bit1, bit2, i, j; 
    unsigned long long int id, mask;
    sav_rom_branch step_branch[32];
    unsigned char len_step_branch;


    len_step_branch = 0;

    i=0;
    mask = 1;
    ret = onewire_reset();
    onewire_write_byte(0xF0);
    id = 0;
    while (i<64)
    {
        bit1 = onewire_read();
        bit2 = onewire_read();
        

        if(bit1 == 0 && bit2 == 0 )
        {
            step_branch[len_step_branch].rom = id;
            step_branch[len_step_branch].len = i;
            len_step_branch++;
            id << 1;
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

    i = 0;
    while(i<len_step_branch)
    {
        id = step_branch[i].rom
        ret = onewire_reset();
        onewire_write_byte(0xF0);
        j = 0;
        while (j != step_branch[i].len)
        {   
            
            bit1 = onewire_read();
            bit2 = onewire_read();
            if(((id >> j) & mask) == 1)
            {onewire_write_one();}
            else{onewire_write_zero();}
            j++;
        }
        onewire_write_one();
        id += mask;
        while(j<64)
        {
            bit1 = onewire_read();
            bit2 = onewire_read();
            

            if(bit1 == 0 && bit2 == 0 )
            {
                step_branch[len_step_branch].rom = id;
                step_branch[len_step_branch].len = j;
                len_step_branch++;
                id << 1;
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
            j++;
        }
        i++;
    }
    return 0;
    // ret = onewire_reset();
    // onewire_write_byte(0xF0);

    // int br1, br2; 
    // int i;
    // char id_char[65];
    // char id_char2[65];
    // unsigned long long int id;
    // unsigned long long int ull_mask;
    // ull_mask = 1; //ull_mask = ull_mask << 63;
    // id = 0;
    // i=0;
    // while (i<64)
    // {
    //     br1 = onewire_read(); 
    //     br2 = onewire_read(); 
    //     // pr_warn("bit read %d %d %d\n",i, br1, br2);
        
    //     id = id << 1;
    //     if(br1 == 1) {
    //         // pr_warn("write 1\n"); 
    //         onewire_write_one();
    //         id += ull_mask;
    //     }
    //     else {
    //         // pr_warn("write 0\n"); 
    //         onewire_write_zero();
    //     }
    //     id_char[i] = '0' + br1;
    //     pr_warn("%llu\n",id); 
    //     i++;
    // }
}

