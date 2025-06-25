#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "include/client.h"

void clear_screen() {
    printf("\033[2J\033[1;1H");
}

void print_banner() {
    printf("\033[1;36m");
    printf("=====================================\n");
    printf("     🛠️  Workshop Job Client CLI     \n");
    printf("=====================================\n");
    printf("\033[0m");
}

void get_job_input(Job *job) {
    print_banner();
    printf("Enter your name: ");
    scanf(" %63[^\n]", job->client_name);

    printf("Enter job type (e.g., weld, paint): ");
    scanf(" %63s", job->job_type);

    do {
        printf("Enter priority (1–5): ");
        scanf("%d", &job->priority);
        if (job->priority < 1 || job->priority > 5)
            printf("❌ Invalid priority. Please enter 1–5.\n");
    } while (job->priority < 1 || job->priority > 5);

    do {
        printf("Enter job duration (seconds): ");
        scanf("%d", &job->duration);
        if (job->duration <= 0)
            printf("❌ Duration must be positive.\n");
    } while (job->duration <= 0);
}

void send_job_to_server(Job *job) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    char buffer[128];

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(1);
    }

    send(sock, job, sizeof(Job), 0);
    printf("\n📤 Job sent to server...\n");

    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    buffer[n] = '\0';
    printf("📨 Server Response: \033[1;32m%s\033[0m\n", buffer);

    close(sock);
}

int main() {
    Job job;
    clear_screen();
    get_job_input(&job);

    printf("\n✅ Job details captured:\n");
    printf("Client Name : %s\n", job.client_name);
    printf("Job Type    : %s\n", job.job_type);
    printf("Priority    : %d\n", job.priority);
    printf("Duration    : %d sec\n", job.duration);

    send_job_to_server(&job);
    return 0;
}
