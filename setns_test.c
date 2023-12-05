#define _GNU_SOURCE
#include <err.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void* child_thread(void *arg)
{
    int global_netns_fd;
    int ret;

    // Network interfaces (New network namespace)
    printf("\nChild process: New network namespace\n");
    system("ip addr show");

    // Get the file descriptor of the target namespace
    global_netns_fd = strtol(arg, NULL, 10);

    // Switch the thread to the target namespace
    ret = setns(global_netns_fd, CLONE_NEWNET);
    if (ret == -1) {
        perror("setns");
        exit(EXIT_FAILURE);
    }

    // Execute a command in the global network namespace
    printf("\nChild process: Global network namespace\n");
    system("ip addr show");
}

int main(int argc, char *argv[])
{
    int global_netns_fd;
    char global_netns_fd_str[12];
    int new_netns_fd;
    pthread_t tid;

    // Network interfaces (Global network namespace)
    printf("\nParent process: Global network namespace\n");
    system("ip addr show");

    // Open the /proc/self/ns/net file of the global network namespace
    global_netns_fd = open("/proc/self/ns/net", O_RDONLY);
    if (global_netns_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Create a new NET namespace and enter it
    if (unshare(CLONE_NEWNET) == -1) {
        perror("unshare");
        exit(EXIT_FAILURE);
    }

    // Open the /proc/self/ns/net file of the new network namespace
    new_netns_fd = open("/proc/self/ns/net", O_RDONLY);
    if (new_netns_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Net interfaces (New network namespace)
    printf("\nParent process: New network namespace\n");
    system("ip addr show");

    // Create a child thread and pass the file descriptor as an argument
    sprintf(global_netns_fd_str, "%d", global_netns_fd);
    pthread_create(&tid, NULL, child_thread, global_netns_fd_str);

    // Wait for the child thread to finish
    pthread_join(tid, NULL);

    // Close the file descriptors
    close(global_netns_fd);
    close(new_netns_fd);

    return 0;
}
