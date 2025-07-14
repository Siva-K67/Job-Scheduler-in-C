/* client/client.c  –  Workshop Job Client CLI */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "jobs.h"      /* ← shared struct */
#include "client.h"

char SERVER_IP[64] = "server";
#define SERVER_PORT 8080

void initialize_server_ip() {
    const char* env_ip = getenv("SERVER_HOST");
    if (env_ip)
        strncpy(SERVER_IP, env_ip, sizeof(SERVER_IP) - 1);
    else
        strcpy(SERVER_IP, "127.0.0.1");  // ← default to localhost
}


void clear_screen(void)
{
    printf("\033[2J\033[1;1H");
}

void print_banner(void)
{
    printf("\033[1;36m");
    printf("=====================================\n");
    printf("     🛠️  Workshop Job Client CLI     \n");
    printf("=====================================\n");
    printf("\033[0m");
}

void get_job_input(Job *job)
{
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

    /* leave job_id & arrival_time zeroed – the server will fill them */
    job->job_id      = 0;
}

/* ─── networking ───────────────────────────────────── */
static void send_job_to_server(const Job *job)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(EXIT_FAILURE); }

    struct sockaddr_in serv = {
        .sin_family = AF_INET,
        .sin_port   = htons(SERVER_PORT)
    };
    
    struct hostent *he = gethostbyname(SERVER_IP);
    if (!he) {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }
    memcpy(&serv.sin_addr, he->h_addr_list[0], he->h_length);


    if (connect(sock, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("connect"); exit(EXIT_FAILURE);
    }

    if (send(sock, job, sizeof(Job), 0) != sizeof(Job)) {
        perror("send"); close(sock); exit(EXIT_FAILURE);
    }
    printf("\n📤 Job sent to server …\n");

    char resp[128] = {0};
    ssize_t n = recv(sock, resp, sizeof(resp) - 1, 0);
    if (n > 0) printf("📨 Server response: \033[1;32m%s\033[0m\n", resp);

    close(sock);
}

int main(void)
{
    Job job = {0};
    initialize_server_ip();
    clear_screen();
    get_job_input(&job);

    printf("\n✅ Job details captured:\n");
    printf("Client Name : %s\n", job.client_name);
    printf("Job Type    : %s\n", job.job_type);
    printf("Priority    : %d\n", job.priority);
    printf("Duration    : %d s\n", job.duration);

    send_job_to_server(&job);
    return 0;
}
