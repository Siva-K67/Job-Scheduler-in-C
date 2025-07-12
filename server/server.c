#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/resource.h>     

#include "jobs.h"              
#include "logger.h"            
#include "technician.h"

#define PORT              8080
#define MAX_JOBS          100
#define TECHNICIAN_COUNT    3

/* ───────────────────────────────────────────────────── priority‑queue ─── */
static Job   job_queue[MAX_JOBS];
static int   job_count     = 0;
static int   next_job_id   = 1;

static pthread_mutex_t queue_mutex   = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  job_available = PTHREAD_COND_INITIALIZER;

/* ─────────────────────────────────────────────── Phase 4: cost engine ─── */
static float get_material_cost(const char *job_type)
{
    FILE *fp = fopen("server/material_costs.txt", "r");
    if (!fp) return 50.0f;           

    char  type[50];
    float cost;
    while (fscanf(fp, "%63s %f", type, &cost) != EOF) {
        if (strcmp(type, job_type) == 0) {
            fclose(fp);
            return cost;
        }
    }
    fclose(fp);
    return 50.0f;                     
}

static float calculate_job_cost(const Job *job)
{
    float labor     = job->duration * 2.0f;        /* ₹2/sec */
    float material  = get_material_cost(job->job_type);
    float surcharge = 0.10f * (labor + material);  /* 10 % */
    return labor + material + surcharge;
}

/* ───────────────────────────────────────────── queue helper ───────────── */
static void add_job(Job job)
{
    pthread_mutex_lock(&queue_mutex);

    /* assign server‑side metadata */
    job.job_id      = next_job_id++;
    job.arrival_time = time(NULL);

    if (job_count >= MAX_JOBS) {
        printf("❌ Queue full, dropping job from %s\n", job.client_name);
        pthread_mutex_unlock(&queue_mutex);
        return;
    }

    /* insert by priority (lower value == higher priority) */
    int i = job_count - 1;
    while (i >= 0 && job_queue[i].priority > job.priority) {
        job_queue[i + 1] = job_queue[i];
        --i;
    }
    job_queue[i + 1] = job;
    ++job_count;

    pthread_cond_signal(&job_available);
    pthread_mutex_unlock(&queue_mutex);
}

/* ─────────────────────────────────── technician worker thread ─────────── */
static void *technician_worker(void *arg)
{
    int tech_id = *((int *)arg);
    free(arg);

    while (1) {
        /* 1) Fetch next job */
        pthread_mutex_lock(&queue_mutex);
        while (job_count == 0)
            pthread_cond_wait(&job_available, &queue_mutex);

        Job job = job_queue[0];
        for (int i = 1; i < job_count; ++i)
            job_queue[i - 1] = job_queue[i];
        --job_count;
        pthread_mutex_unlock(&queue_mutex);

        /* 2) Metrics – start snapshot */
        struct timespec ts_start;
        struct rusage   ru_start;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        getrusage(RUSAGE_SELF, &ru_start);

        double wait_secs = difftime(time(NULL), job.arrival_time);
        printf("🔧 Tech %d processing job %d from %s (%s, pri %d, %ds) [wait %.0fs]\n",
               tech_id, job.job_id, job.client_name, job.job_type,
               job.priority, job.duration, wait_secs);

        /* 3) Simulate work */
        sleep(job.duration);

        /* 4) Metrics – end snapshot */
        struct timespec ts_end;
        struct rusage   ru_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        getrusage(RUSAGE_SELF, &ru_end);

        double exec_secs = (ts_end.tv_sec - ts_start.tv_sec) +
                           (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;
        double cpu_user  = (ru_end.ru_utime.tv_sec +
                            ru_end.ru_utime.tv_usec / 1e6) -
                           (ru_start.ru_utime.tv_sec +
                            ru_start.ru_utime.tv_usec / 1e6);
        double cpu_sys   = (ru_end.ru_stime.tv_sec +
                            ru_end.ru_stime.tv_usec / 1e6) -
                           (ru_start.ru_stime.tv_sec +
                            ru_start.ru_stime.tv_usec / 1e6);

        /* 5) Cost + console */
        float cost = calculate_job_cost(&job);
        printf("✅ Tech %d completed job %d | cost ₹%.2f | exec %.1fs | CPU u%.2f s%.2f\n",
               tech_id, job.job_id, cost, exec_secs, cpu_user, cpu_sys);

        /* 6) Phase 5 – persistent logs */
        log_job_completion(&job, tech_id, cost,
                           wait_secs, exec_secs, cpu_user, cpu_sys);
    }
    return NULL;
}

int main(void)
{
    int                 server_fd, client_fd;
    struct sockaddr_in  addr;
    socklen_t           addrlen = sizeof(addr);
    Job                 job;

    /* 0) Start logger immediately */
    init_logger();

    /* 1) Spawn technician pool */
    for (int i = 0; i < TECHNICIAN_COUNT; ++i) {
        pthread_t tid;
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&tid, NULL, technician_worker, id);
        pthread_detach(tid);
    }

    /* 2) Set up listening socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    printf("🟢 Server listening on port %d …\n", PORT);

    /* 3) Accept & enqueue loop */
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            perror("accept"); continue;
        }

        ssize_t n = recv(client_fd, &job, sizeof(Job), 0);
        if (n == sizeof(Job)) {
            printf("📥 Received job from %s\n", job.client_name);
            add_job(job);

            const char resp[] = "✅ Job received & queued!";
            send(client_fd, resp, sizeof(resp), 0);
        } else {
            fprintf(stderr, "⚠️  Partial / invalid job received\n");
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
