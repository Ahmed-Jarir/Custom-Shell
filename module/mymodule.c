#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define DEVICE_NAME "node_device"
#define CLASS_NAME "node"

// Meta Information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A module that knows how to greet");

/* char *name; */
/* int age; */
int pid;

/*
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is its data type
 * The final argument is the permissions bits,
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 */

/* module_param(name, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); */
/* MODULE_PARM_DESC(name, "name of the caller"); */

/* module_param(age, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); */
/* MODULE_PARM_DESC(age, "age of the caller"); */

module_param(pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(pid, "PID of the process");

struct node {
    int process_pid;
    int parent_pid;
    unsigned long creation_time;
    int eldest_child_index;
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
    parent->parent_pid = -1;
    parent->eldest_child_index = 0;
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
        child_node->parent_pid = Pid;
        parent->children[i] = child_node;
        if(parent->children[i]->creation_time < parent->children[parent->eldest_child_index]->creation_time){
            parent->eldest_child_index = i;
        }
        i++;
    }

    return parent;
}

void printTree(struct node* tree) {
    printk(KERN_INFO "pid: %d, ppid: %d, creation_time: %lu, eldest child: %d\n", tree->process_pid, tree->parent_pid, tree->creation_time, tree->number_of_children == 0 ? 0 : tree->children[tree->eldest_child_index]->process_pid);
    if(tree->number_of_children == 0) {
        return;
    }
    for (int i = 0; i < tree->number_of_children; i++){
        printTree(tree->children[i]);
    }

}


int simple_init(void) {
    printk("Hello from the kernel\n");
    if(pid) {
        struct node* tree = get_children_of_process(pid);
        printTree(tree);
    }
    return 0;
}

// A function that runs when the module is removed
void simple_exit(void) {
	printk("Goodbye from the kernel\n");
}

module_init(simple_init);
module_exit(simple_exit);
