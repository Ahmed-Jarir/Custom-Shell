
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/string.h>
#include <linux/timer.h>

#define NETLINK_USER 31

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A module that knows how to greet");

int PID;
module_param(PID, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(PID, "PID of the process");
struct my_struct {
    int process_pid;
    int parent_pid;
    unsigned long creation_time;
    int eldest_child;
    int num_of_nodes;
    int number_of_children;
};

struct node {
    int process_pid;
    int parent_pid;
    unsigned long creation_time;
    int eldest_child_index;
    int eldest_child;
    int number_of_children;
    struct node **children;
};
struct sock *nl_sk = NULL;
struct timer_list my_timer;
unsigned int pid;

static void my_input(struct sk_buff *skb) {
    struct nlmsghdr *nlh;

    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid; // Save the user-space process ID (PID)

    printk(KERN_INFO "Received message from user-space with pid: %u\n", pid);
}

int num_of_nodes = 1;
struct node* get_children_of_process(int Pid)
{
    struct task_struct *task;
    struct task_struct *child;
    struct node *parent = NULL;
    int num_of_children = 0;
    int i;
    int first_child = 1;

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
    parent->eldest_child = 0;
    parent->eldest_child_index = 0;
    parent->creation_time = task->start_time;
    parent->number_of_children = num_of_children;

    // allocate memory for the children array
    if(num_of_children != 0) {
        parent->children = kmalloc(num_of_children * sizeof(struct node*), GFP_USER);
        if (parent->children == NULL) {
            kfree(parent);
            return NULL;
        }
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
        if(first_child) {
            parent->children[i]->eldest_child = 1;
            parent->eldest_child_index = i;
            first_child = 0;
        } else if(parent->children[i]->creation_time < parent->children[parent->eldest_child_index]->creation_time) {
            parent->children[parent->eldest_child_index]->eldest_child = 0;
            parent->children[i]->eldest_child = 1;
            parent->eldest_child_index = i;
        }
        /* if(parent->children[i]->creation_time <= parent->children[parent->eldest_child_index]->creation_time){ */
        /*     parent->children[parent->eldest_child_index]->eldest_child = 0; */
        /*     parent->children[i]->eldest_child = 1; */
        /*     parent->eldest_child_index = i; */
        /* } else { */

        /* } */
        num_of_nodes++; 
        i++;
    }

    return parent;
}

void sendTree(struct node* tree) {
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    struct my_struct data;
    int res;   

    data.process_pid = tree->process_pid;
    data.parent_pid = tree->parent_pid;
    data.creation_time = tree->creation_time;
    data.number_of_children = tree->number_of_children;
    data.eldest_child = tree->eldest_child;
    data.num_of_nodes = num_of_nodes;

    skb_out = nlmsg_new(sizeof(data), 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, sizeof(data), 0);
    memcpy(nlmsg_data(nlh), &data, sizeof(data));

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0) {
        printk(KERN_INFO "Error while sending message to user: %d\n", res);
    } else {
        printk(KERN_INFO "Message sent to user.\n");
    }

    mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));


    if(tree->number_of_children == 0) {
        return;
    }
    for (int i = 0; i < tree->number_of_children; i++){
        sendTree(tree->children[i]);
    }

}
void printTree(struct node* tree) {
    printk(KERN_INFO "pid: %d, ppid: %d, creation_time: %lu, eldest child: %d\n", tree->process_pid, tree->parent_pid, tree->creation_time, tree->eldest_child);
    if(tree->number_of_children == 0) {
        return;
    }
    for (int i = 0; i < tree->number_of_children; i++){
        printTree(tree->children[i]);
    }

}
static void send_to_userspace(struct timer_list *t) {

    struct node* tree;

    if (pid == 0) {
        mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));
        return;
    }

    num_of_nodes = 1;
    tree = get_children_of_process(PID);
    sendTree(tree);
    // uncommment to check tree structure in kernel
    /* printTree(tree); */

}

static int __init my_module_init(void) {
    struct netlink_kernel_cfg cfg = {
        .input = my_input,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating Netlink socket.\n");
        return -10;
    }

    printk(KERN_INFO "Netlink socket created.\n");

    timer_setup(&my_timer, send_to_userspace, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));

    return 0;
}

static void __exit my_module_exit(void) {
    del_timer(&my_timer);
    netlink_kernel_release(nl_sk);
    printk(KERN_INFO "Netlink socket released.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

