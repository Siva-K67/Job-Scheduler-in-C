// server/server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_STR 64

typedef struct {
    char client_name[MAX_STR];
    char job_type[MAX_STR];
    int priority;
    int duration;
} Job;

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    Job job;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket failed"); exit(1); }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    printf("🟢 Server waiting on port %d...\n", PORT);
    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    printf("🔗 Client connected!\n");

    recv(new_socket, &job, sizeof(Job), 0);
    printf("📥 Received job from %s: %s, priority %d, duration %d\n",
        job.client_name, job.job_type, job.priority, job.duration);
    char response[] = "✅ Job received successfully by server!";
    int len = strlen(response) + 1;
    send(new_socket, response, len, 0);

    close(new_socket);
    close(server_fd);
    return 0;
}

