#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_STR 64
#define MAX_JOBS 100
#define TECHNICIAN_COUNT 3

typedef struct {
    char client_name[MAX_STR];
    char job_type[MAX_STR];
    int priority;
    int duration;
    time_t arrival_time;
} Job;

Job job_queue[MAX_JOBS];
int job_count = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t job_available = PTHREAD_COND_INITIALIZER;

void add_job(Job job) {
    pthread_mutex_lock(&queue_mutex);
    if (job_count >= MAX_JOBS) {
        printf("❌ Queue full, dropping job from %s\n", job.client_name);
        pthread_mutex_unlock(&queue_mutex);
        return;
    }

    job.arrival_time = time(NULL);
    int i = job_count - 1;
    while (i >= 0 && job_queue[i].priority > job.priority) {
        job_queue[i + 1] = job_queue[i];
        i--;
    }
    job_queue[i + 1] = job;
    job_count++;

    pthread_cond_signal(&job_available);
    pthread_mutex_unlock(&queue_mutex);
}

void* technician_worker(void* arg) {
    int id = *((int*)arg);
    free(arg);

    while (1) {
        pthread_mutex_lock(&queue_mutex);
        while (job_count == 0)
            pthread_cond_wait(&job_available, &queue_mutex);

        Job job = job_queue[0];
        for (int i = 1; i < job_count; i++)
            job_queue[i - 1] = job_queue[i];
        job_count--;

        pthread_mutex_unlock(&queue_mutex);

        double wait_time = difftime(time(NULL), job.arrival_time);
        printf("🔧 Technician %d is processing job: %s (%s, priority %d, duration %ds) [Wait: %.0fs]\n",
               id, job.client_name, job.job_type, job.priority, job.duration, wait_time);

        sleep(job.duration);

        printf("✅ Technician %d completed job from %s\n", id, job.client_name);
    }

    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    Job job;

    for (int i = 0; i < TECHNICIAN_COUNT; i++) {
        pthread_t tid;
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&tid, NULL, technician_worker, id);
        pthread_detach(tid);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    printf("🟢 Server running on port %d. Waiting for jobs...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) { perror("accept"); continue; }

        recv(new_socket, &job, sizeof(Job), 0);
        printf("📥 Received job from %s\n", job.client_name);
        add_job(job);

        char response[] = "✅ Job received & queued!";
        send(new_socket, response, strlen(response) + 1, 0);

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
