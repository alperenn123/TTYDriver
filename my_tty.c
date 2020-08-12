#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/serial_reg.h>

#define DRIVER_AUTHOR "Alperen"
#define DRIVER_DESC "TTY Driver"


MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE("GPL");


#define MY_TTY_MAJOR 255
#define MY_TTY_MINOR 1


#define HW_SIZE 128

struct mytty_serial{
  struct tty_struct *tty;
  struct file *filp;
  int open_count;
  struct semaphore sem;
  char *hw_buffer;
};



static struct mytty_serial *mytty_serial = NULL;



static int mytty_open(struct tty_struct *tty, struct file *filp)
{
  
  printk(KERN_INFO "%s\n",__func__);

  tty->driver_data = NULL;

  if (NULL == mytty_serial){
    mytty_serial = kmalloc(sizeof(*mytty_serial),GFP_KERNEL);
    if ( NULL == mytty_serial){
      return -ENOMEM;
    }
    mytty_serial->hw_buffer = kmalloc(HW_SIZE,GFP_KERNEL);
    if (!mytty_serial->hw_buffer){
      kfree(mytty_serial);
      return -ENOMEM;
    }
    sema_init(&mytty_serial->sem,1);
    mytty_serial->open_count = 0;
  }
  down(&mytty_serial->sem);
  tty->driver_data = mytty_serial;
  mytty_serial->tty = tty;
  mytty_serial->filp = filp;

  ++mytty_serial->open_count;
  /*if(1 == mytty_serial->open_count){
    mytty_serial->head = 0;
    mytty_serial->tail = 0;;
    }*/
  up(&mytty_serial->sem);

  return 0;
}

static void do_close(struct mytty_serial *my_serial)
{
  printk(KERN_INFO "%s\n",__func__);

  down(&my_serial->sem);

  if (0 == my_serial->open_count) return;

  --my_serial->open_count;

  up(&my_serial->sem);

}

static void mytty_close(struct tty_struct *tty, struct file *filp)
{
  struct mytty_serial *my_serial;
  my_serial = tty->driver_data;
  printk(KERN_INFO "%s \n",__func__);
  if (!my_serial) return;

  if (my_serial) do_close(my_serial);
}

static int mytty_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
  struct mytty_serial *mytty;
  int ret_val = -EINVAL;
  int c,c1;
  
  mytty = tty->driver_data;
  if (!mytty){
    return -ENODEV;
  }

  down(&mytty->sem);

  if (!mytty->open_count){
    up (&mytty->sem);
    return -EINVAL;
  }
  printk(KERN_INFO "%s char = %c\n",__func__,buf[0]);
  c1 = 0;
  for(c = 0; c<count; c++){
    mytty->hw_buffer[c1] = buf[c];
    c1 = (c1 + 1) % HW_SIZE;
    tty_insert_flip_char(tty->port,buf[c],TTY_NORMAL);
  }
  tty_flip_buffer_push(tty->port);
  ret_val = count;
  up(&mytty->sem);
  return ret_val;
}


static int mytty_write_room(struct tty_struct *tty)
{
  struct mytty_serial *mytty = tty->driver_data;
  int room;
  if (!mytty) return -ENODEV;

  down(&mytty->sem);
  if (!mytty->open_count){
    return -EINVAL;
  }
  up(&mytty->sem);
  room = 255;
  return room;
}

static int mytty_install(struct tty_driver *driver, struct tty_struct *tty)
{
  printk(KERN_INFO "%s\n",__func__);

  tty->port = kmalloc(sizeof(*tty->port),GFP_KERNEL);
  if (!tty->port){
    return -ENOMEM;
  }

  tty_init_termios(tty);
  driver->ttys[0] = tty;

  tty_port_init(tty->port);
  tty_buffer_set_limit(tty->port, 8192);
  tty_driver_kref_get(driver);
  tty->count++;


  return 0;
}

static void mytty_set_termios(struct tty_struct * tty, struct ktermios *old_termios)
{
  printk(KERN_INFO "%s\n",__func__);
}
static struct tty_operations serial_ops = {
  .open = mytty_open,
  .close = mytty_close,
  .write = mytty_write,
  .write_room = mytty_write_room,
  .set_termios = mytty_set_termios,
  .install = mytty_install
};

static struct tty_driver *mytty_tty_driver;

static int __init mytty_init(void)
{

  mytty_tty_driver = alloc_tty_driver(MY_TTY_MINOR);

  if (!mytty_tty_driver) return -ENOMEM;

  mytty_tty_driver->magic = TTY_DRIVER_MAGIC;
  mytty_tty_driver->owner = THIS_MODULE;
  mytty_tty_driver->driver_name = "mytty_driver";
  mytty_tty_driver->name = "mytty";
  mytty_tty_driver->major = MY_TTY_MAJOR;
  mytty_tty_driver->num = MY_TTY_MINOR;
  mytty_tty_driver->type = TTY_DRIVER_TYPE_SYSTEM;
  mytty_tty_driver->subtype = SYSTEM_TYPE_CONSOLE;
  mytty_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV | TTY_DRIVER_UNNUMBERED_NODE;
  mytty_tty_driver->init_termios = tty_std_termios;
  mytty_tty_driver->init_termios.c_cflag = B115200 | CS8 | CREAD | HUPCL |CLOCAL;
  mytty_tty_driver->init_termios.c_lflag &= ~ECHO;
  tty_set_operations(mytty_tty_driver,&serial_ops);

  

  if (tty_register_driver(mytty_tty_driver)){
    printk(KERN_ERR "failed to register mytty driver \n");
    put_tty_driver (mytty_tty_driver);
    return -1;
  }
  tty_register_device(mytty_tty_driver,0,NULL);
  printk(KERN_INFO "Mytty driver is running \n");
  return 0;
}

static void __exit mytty_exit(void)
{

  printk(KERN_INFO "%s\n",__func__);

  tty_unregister_device(mytty_tty_driver,0);
  tty_unregister_driver(mytty_tty_driver);

  if (mytty_serial){
    while(mytty_serial->open_count){
      do_close(mytty_serial);
    }
    if (mytty_serial->tty->port){
      kfree(mytty_serial->tty->port);
      mytty_serial->tty->port = NULL;
    }
    kfree(mytty_serial);
    mytty_serial = NULL;
  }
  printk(KERN_INFO "Mytty driver exit\n");
}

module_init(mytty_init);
module_exit(mytty_exit);
