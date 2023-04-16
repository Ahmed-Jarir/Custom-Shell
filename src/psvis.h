#ifndef PSVIS
#define PSVIS
#include "global.h"
#include "findFile.h"

#define NETLINK_USER 31

// structure that we use to store our response from the kernel module
struct my_struct {
    int process_pid;
    int parent_pid;
    unsigned long creation_time;
    int eldest_child;
    int numOfNodes;
    int number_of_children;
};

// creates node of the tree
void add_nodes(struct my_struct *nodes, int num_nodes, Agraph_t *graph) {
    for (int i = 0; i < num_nodes; ++i) {
        char label[256];
        snprintf(label, sizeof(label), "PID: %d\nCreation Time: %lu", nodes[i].process_pid, nodes[i].creation_time);

        Agnode_t *node = agnode(graph, label, 1);
        if (nodes[i].eldest_child) {
            agset(node, "color", "red");
        }
    }
}

// adds edges between the nodes of the tree 
void add_edges(struct my_struct *nodes, int num_nodes, Agraph_t *graph) {
    for (int i = 0; i < num_nodes; ++i) {
        char parent_label[256], child_label[256];
        snprintf(parent_label, sizeof(parent_label), "PID: %d\nCreation Time: %lu", nodes[i].process_pid, nodes[i].creation_time);

        Agnode_t *parent_node = agnode(graph, parent_label, 0);

        if (parent_node == NULL) {
            printf("Error: could not find parent node with label: %s\n", parent_label);
            continue;
        }

        for (int j = 0; j < num_nodes; ++j) {
            if (nodes[j].parent_pid == nodes[i].process_pid) {
                snprintf(child_label, sizeof(child_label), "PID: %d\nCreation Time: %lu", nodes[j].process_pid, nodes[j].creation_time);

                Agnode_t *child_node = agnode(graph, child_label, 0);

                if (child_node == NULL) {
                    printf("Error: could not find child node with label: %s\n", child_label);
                    continue;
                }

                Agedge_t *edge = agedge(graph, parent_node, child_node, 0, 1);

                if (edge == NULL) {
                    printf("Error: could not create edge between parent: %s and child: %s\n", parent_label, child_label);
                }
            }
        }
    }
}
// fetches the data from the kernel
int getdata(char* outputfile) {

    int sock_fd;
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh;
    struct msghdr msg;
    struct iovec iov;

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; // Send to kernel

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct my_struct)));
    memset(nlh, 0, NLMSG_SPACE(sizeof(struct my_struct)));
    nlh->nlmsg_len = NLMSG_SPACE(sizeof(struct my_struct));
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sendmsg(sock_fd, &msg, 0);

    printf("Waiting for message from kernel...\n");


    struct my_struct nodes[1024];
    int node_count = 0;

    while (1) {
        // Receive the message from the kernel
        recvmsg(sock_fd, &msg, 0);

        struct my_struct *received_data = (struct my_struct *)NLMSG_DATA(nlh);
        nodes[node_count] = *received_data;

        node_count++;

        if (received_data->numOfNodes <= node_count) {
            break;
        }
    }

    close(sock_fd);
    free(nlh);



    Agraph_t *graph = agopen("process_tree", Agdirected, 0);
    agattr(graph, AGNODE, "color", "black");

    add_nodes(nodes, node_count, graph);

    add_edges(nodes, node_count, graph);

    // opens a dot file which is the file format that graphviz uses
    FILE *output = fopen("output.dot", "w");
    // writes the data from the graph created above into the dot file
    agwrite(graph, output);

    fclose(output);

    agclose(graph);
    char cmd[strlen(outputfile) + 35];
    // creates a command to dump the dot file data into a png file with the name given from the arguments
    sprintf(cmd,"dot -Tpng output.dot -o %s.png",outputfile);
    // runs the command
    if(system(cmd)) {
        return 1;
    }
    // removes the dot file after getting the png file
    if(system("rm output.dot")) {
        return 1;
    }

    return 0;
}
// loads the module using insmod 
// Note: the kernel module has to be compiled
int load_module(char* module_path, int pid)
{
    if (system(NULL) == 0) {
        fprintf(stderr, "No shell is available to execute the command\n");
        return 1;
    }

    char command[256];
    snprintf(command, sizeof(command), "sudo insmod %s PID=%d", module_path, pid);

    int result = system(command);
    if (result != 0) {
        return 1;
    }

    return 0;
}
// unloads the module
int unload_module(const char* module_name) {
    if (system(NULL) == 0) {
        fprintf(stderr, "No shell is available to execute the command\n");
        return 1;
    }

    char command[256];
    snprintf(command, sizeof(command), "sudo rmmod %s", module_name );

    int result = system(command);
    if (result != 0) {
        return 1;
    }

    return 0;
}

int psvis(int pid, char* outputfile) {
    // the relative path of the compiled module relative to our project directory 
    const char* mod_file = "module/mymodule.ko";
    // name of the module to unload it
    const char* mod_name = "mymodule";

    char* module_path = find_file(mod_file);
    if (load_module(module_path, pid)) return 1;
    
    int code = getdata(outputfile);

    if (unload_module(mod_name)) return 1;

    return code;
}
#endif
