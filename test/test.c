
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NETLINK_USER 31


struct my_struct {
    int process_pid;
    int parent_pid;
    unsigned long creation_time;
    int eldest_child_index;
    int numOfNodes;
    int number_of_children;
};

int main() {
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

    printf("Sending initial message to kernel...\n");
    sendmsg(sock_fd, &msg, 0);
    printf("Initial message sent.\n");

    printf("Waiting for message from kernel...\n");

    int loops = 0;
    while (1) {
        // Receive the message from the kernel
        recvmsg(sock_fd, &msg, 0);

        struct my_struct *received_data = (struct my_struct *)NLMSG_DATA(nlh);
        printf("Received message payload: pid: %d, ppid: %d, creation time: %lu, eldest: %d numochild: %d numOfNodes: %d\n",
                received_data->process_pid,
                received_data->parent_pid,
                received_data->creation_time,
                received_data->eldest_child_index,
                received_data->number_of_children,
                received_data->numOfNodes
                );
        loops++;
        printf("loops: %d\n",loops);

    }
    printf("");


    close(sock_fd);
    free(nlh);

    return 0;
}
