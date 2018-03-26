#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grigoriev D.S.");
MODULE_DESCRIPTION("test task");
MODULE_VERSION("0.01");

// для инициализации метафайла в директории /dev
#define CLASS_NAME "qik"
#define DEVICE_NAME "qikchar"

static struct class *QIKC = NULL;
static struct device *QIKD = NULL;


// для работы с очередью
#define SIZE_STR_QUEUE (64*1024)
#define MAX_NUM_QUEUE_ITEMS 1024

static struct Queue_Item
{
    char str[SIZE_STR_QUEUE];
    struct Queue_Item *next_item;
} *Front_Item, *Back_Item;

static unsigned int Number_Queue_Items;

//--------------------------------------------------------------------------------------------
// Прототипы ввода вывода
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);

static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int major_num;
static int device_open_count = 0;
//--------------------------------------------------------------------------------------------
// Структура с указателями на все функции устройства
static struct file_operations file_ops = {
 .read = device_read,
 .write = device_write,
 .open = device_open,
 .release = device_release
};
//--------------------------------------------------------------------------------------------
// вызывается при чтении из устройства
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) 
{
    int bytes_read =MAX_NUM_QUEUE_ITEMS;
    struct Queue_Item *item;
    
    if(Number_Queue_Items==0)
    {
        printk(KERN_INFO "queue_in_kernel: queue is empty\n");
        return 0;
    }
    
    if(!access_ok(VERIFY_WRITE, buffer, len))
    {
        printk(KERN_ALERT "queue_in_kernel: access to memory is prohibited\n");
        return -EINVAL;
    }
    
    item=Front_Item;
    Front_Item=item->next_item;
    while(bytes_read)
    {
        if(put_user(*(item->str+(MAX_NUM_QUEUE_ITEMS-bytes_read)), buffer++))
        {
            printk(KERN_ALERT "queue_in_kernel: error read\n");
            return -EFAULT;
        }
        bytes_read--;
    }
    
    kfree(item);
    Number_Queue_Items--;
 
    return MAX_NUM_QUEUE_ITEMS;
}
//--------------------------------------------------------------------------------------------
// Вызывается при записи в устройство
static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) 
{
    int bytes_write = SIZE_STR_QUEUE;
    struct Queue_Item *item;
  
    if(Number_Queue_Items==MAX_NUM_QUEUE_ITEMS)
    {
        printk(KERN_INFO "queue_in_kernel: queue is full\n");
        return 0;
    }
    
    if(!access_ok(VERIFY_READ, buffer, len))
    {
        printk(KERN_ALERT "queue_in_kernel: access to memory is prohibited\n");
        return -EINVAL;
    }
    
    item = (struct Queue_Item *)kmalloc(sizeof(struct Queue_Item), GFP_USER);
    
    while(bytes_write)
    {
        item->str[--bytes_write]=0;
    }
        
    if((len+1)>SIZE_STR_QUEUE)
    {
        len=SIZE_STR_QUEUE;
    }

    while (len) 
    {
        
        if(get_user(*(item->str+bytes_write), buffer++))
        {
            printk(KERN_ALERT "queue_in_kernel: error write\n");
            return -EFAULT;
        }
        len--;
        bytes_write++;
    }
    item->next_item=NULL;
    
    if(Number_Queue_Items==0)
    {
        Front_Item = item;
        Back_Item = item;
    }
    else
    {
        Back_Item->next_item=item;
        Back_Item=item;

    }
    
    Number_Queue_Items++;
    
    printk(KERN_INFO "queue_in_kernel: add %s number %d\n", item->str, Number_Queue_Items);
    
    return bytes_write;
}
//--------------------------------------------------------------------------------------------
// Вызывается при открытии устройства
static int device_open(struct inode *inode, struct file *file) 
{
    if (device_open_count==2) 
    {
        return -EBUSY;
    }
    device_open_count++;
    try_module_get(THIS_MODULE);
    return 0;
}
//--------------------------------------------------------------------------------------------
// Вызывается при закрытии устройства
static int device_release(struct inode *inode, struct file *file) 
{
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}
//--------------------------------------------------------------------------------------------
// функция инициализации
static int __init Queue_init(void) 
{

    major_num = register_chrdev(0, "queue_in_kernel", &file_ops);
    if (major_num < 0) 
    {
        printk(KERN_ALERT "queue_in_kernel: failed to register a major number\n");
        return major_num;
    } 
    printk(KERN_INFO "queue_in_kernel: registered correctly with major number %d\n", major_num);
    
    
    QIKC = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(QIKC))
    {
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "queue_in_kernel: Failed to register device class\n");
        return PTR_ERR(QIKC);
    }
    printk(KERN_INFO "queue_in_kernel: device class registered correctly\n");
    
    
    
    QIKD = device_create(QIKC, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
    if (IS_ERR(QIKD))
    {
        class_destroy(QIKC);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "queue_in_kernel: Failed to create the device\n");
        return PTR_ERR(QIKD);
    }
    printk(KERN_INFO "queue_in_kernel: qikchar: device class created correctly\n");
        
    Front_Item=NULL;
    Back_Item=NULL; 
    Number_Queue_Items=0;
    return 0;
}
//--------------------------------------------------------------------------------------------
// функция выхода
static void __exit Queue_exit(void) 
{
    struct Queue_Item *item;
    device_destroy(QIKC, MKDEV(major_num, 0));
    class_unregister(QIKC);
    class_destroy(QIKC);
    unregister_chrdev(major_num, DEVICE_NAME);

    //Очистка памяти от данных
    while(Number_Queue_Items)
    {
        item=Front_Item;
        Front_Item=item->next_item;
        kfree(item);
        Number_Queue_Items--;
        Back_Item=NULL;
    }
    printk(KERN_INFO "queue_in_kernel: queue clear\n");
    
    
    // delete all file
    
    
    printk(KERN_INFO "queue_in_kernel: Goodbye, World!\n");
}
//--------------------------------------------------------------------------------------------
// передача функций входа и выхода
module_init(Queue_init);
module_exit(Queue_exit);