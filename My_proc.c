#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <linux/buffer_head.h>
#define BUFSIZE  100


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");

#define MAX 256
static char message[MAX] =""; 

static char proc_data[16384] = {};

static struct proc_dir_entry *ent;

static ssize_t mywrite(struct file *file, const char __user *buffer,size_t length, loff_t *ppos) 
{	
	pid_t mypid;
	struct task_struct *theTask;
	char thisMessage[256];
	thisMessage[0] = '\0';
	mypid = current->pid;

	copy_from_user(message, buffer, length);

	if(strcmp(message, "clear") == 0)
	{
		memset(proc_data,'\0',16384);
	}

	theTask = pid_task(find_vpid(mypid), PIDTYPE_PID);

	sprintf(thisMessage, "\tThreadID:%d Time:%lld(ms) context switch times: %ld\n", mypid, (theTask->utime)/1000000, theTask->nvcsw + theTask->nivcsw );

	printk( KERN_INFO "%ld %ld %lld\n",theTask->nvcsw, theTask->nivcsw, theTask->utime);

	strcat(proc_data,thisMessage);

	printk( KERN_INFO "write handler %d",mypid);
	printk(KERN_INFO "Received %s characters from the user\n", message);
	return 0;
}

static ssize_t myread(struct file* file, char __user* user_buffer, size_t length, loff_t* offset)
{

	 if (*offset > 0)
	 {
	   return 0;
	 }
	 copy_to_user(user_buffer, proc_data, strlen(proc_data));
	 *offset =  strlen(proc_data);
	 return strlen(proc_data);
}

static struct file_operations myops = 
{
	.owner = THIS_MODULE,
	.read = myread,
	.write = mywrite
};

static int simple_init(void)
{
	ent=proc_create("thread_info",0666,NULL,&myops);
	return 0;
}

static void simple_cleanup(void)
{
	proc_remove(ent);
}

module_init(simple_init);
module_exit(simple_cleanup);
