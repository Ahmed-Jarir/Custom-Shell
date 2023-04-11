#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>

// Meta Information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A module that knows how to greet");

char *name;
int age;
int pid;

/*
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is its data type
 * The final argument is the permissions bits,
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 */

module_param(name, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(name, "name of the caller");

module_param(age, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(age, "age of the caller");

module_param(pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(pid, "PID of the process");

struct node {
    int process_pid;
    unsigned long creation_time;
    int number_of_children;
    struct node **children;
};


// a function to get the children of a process given its PID
struct node* get_children_of_process(int Pid)
{
    struct task_struct *task;
    struct task_struct *child;
    struct node *parent = NULL;
    int num_of_children = 0;
    int i;

    // find the task_struct associated with the given PID
    task = pid_task(find_vpid(Pid), PIDTYPE_PID);

    if (task == NULL) {
        printk(KERN_INFO "No process found with PID: %d\n", Pid);
        return NULL;
    }

    // get the number of children
    list_for_each_entry(child, &task->children, sibling) {
        num_of_children++;
    }

    // allocate memory for the parent node
    parent = kmalloc(sizeof(struct node), GFP_USER);
    if (parent == NULL) {
        return NULL;
    }

    // initialize the parent node
    parent->process_pid = Pid;
    parent->creation_time = task->start_time;
    parent->number_of_children = num_of_children;

    // allocate memory for the children array
    parent->children = kmalloc(num_of_children * sizeof(struct node*), GFP_USER);
    if (parent->children == NULL) {
        kfree(parent);
        return NULL;
    }

    // recursively add children array
    i = 0;
    list_for_each_entry(child, &task->children, sibling) {
        struct node *child_node = get_children_of_process(child->pid);
        if (child_node == NULL) {
            // Free the parent node and return NULL on error
            kfree(parent->children);
            kfree(parent);
            return NULL;
        }
        parent->children[i] = child_node;
        i++;
    }

    return parent;
}

// A function that runs when the module is first loaded
int simple_init(void) {
	struct task_struct *ts;
    struct node* tree;
	ts = get_pid_task(find_get_pid(4), PIDTYPE_PID);
    tree = get_children_of_process(pid);

    if (tree != NULL){
        printk(KERN_INFO "root PID: %d, Start Time: %lu\n", tree->process_pid, tree->creation_time);
        for(int i = 0; i < tree->number_of_children; i++){
            struct node* child = tree->children[i];
            printk(KERN_INFO "Child PID: %d, Start Time: %lu\n", child->process_pid, child->creation_time);

        }
    }
	printk("Hello from the kernel, user: %s, age: %d\n", name, age);
	printk("command: %s\n", ts->comm);
	return 0;
}

// A function that runs when the module is removed
void simple_exit(void) {
	printk("Goodbye from the kernel, user: %s, age: %d\n", name, age);
}



module_init(simple_init);
module_exit(simple_exit);
