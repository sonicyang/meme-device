#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "meme" // Device name
#define BUF_LEN 120 // Max length of a meme
#define MEME_NUM 12 // Number of memes

static int major; //Major number assigned
static int device_read_count = 0; //Current meme
static char msg[MEME_NUM][BUF_LEN]; // Meme database

// Register the functions for a file device
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

//Entry point of the kmod
int init_module(void)
{
    //Register a char device
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
      printk(KERN_ALERT "Registering meme device failed with %d\n", Major);
      return major;
    }

    //Print some info message with usage
    printk(KERN_INFO "Meme device is assigned major number %d. To talk to\n", Major);
    printk(KERN_INFO "the device, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major);
    printk(KERN_INFO "Meme can be get from reading the device.\n");

    return SUCCESS;
}

//function called on unloading
void cleanup_module(void){
    /*
     * Unregister the device
     */
    int ret = unregister_chrdev(major, DEVICE_NAME);
    if (ret < 0)
        printk(KERN_ALERT "Error in unregister_meme device: %d\n", ret);
}

//This function is called upon file opening
static int device_open(struct inode *inode, struct file *file){
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

//This function is called upon file closing
static int device_release(struct inode *inode, struct file *file){
    module_put(THIS_MODULE);

    return 0;
}

//Handler for reading the file, should return the meme
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
    int bytes_read = 0;
    char *ptr = msg[device_read_count++];

    // Wrap back is > 12 reads
    if(device_read_count >= 12)
        device_read_count = 0;

    // Transfer the msg to buffer
    while (length && *msg_Ptr) {
        //User space memory are not directly accessible. Thus, we use put_user
        put_user(*(ptr++), buffer++);

        length--;
        bytes_read++;
    }

    // Return the number of bytes read
    return bytes_read;
}

// Device writing handler
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
    return -EINVAL;
}
