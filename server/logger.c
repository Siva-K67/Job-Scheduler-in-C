#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "logger.h"
#include "jobs.h"

/* Locations */
#define LOG_DIR        "logs"
#define JOB_LOG        "logs/jobs.log"      // human‑readable
#define METRIC_CSV     "logs/metrics.csv"   // machine‑friendly

static FILE *f_job, *f_csv;

static int file_exists(const char *path)
{
    struct stat sb;
    return stat(path, &sb) == 0;
}

void init_logger(void)
{
    /* Ensure logs/ exists */
    mkdir(LOG_DIR, 0755);

    /* 1) Append‑mode text log */
    f_job = fopen(JOB_LOG, "a");
    if (!f_job) { perror("open jobs.log"); exit(1); }

    /* 2) CSV – add header only once */
    int new_file = !file_exists(METRIC_CSV);
    f_csv = fopen(METRIC_CSV, "a");
    if (!f_csv) { perror("open metrics.csv"); exit(1); }

    if (new_file) {
        fprintf(f_csv,
          "timestamp,job_id,client,job_type,priority,duration,"
          "technician,wait_s,exec_s,cost_rupees,cpu_user_s,cpu_sys_s\n");
        fflush(f_csv);
    }
}

void log_job_completion(const Job *job,
                        int technician_id,
                        float cost,
                        double wait_time,
                        double exec_time,
                        double cpu_user,
                        double cpu_sys)
{
    /* Pretty log */
    fprintf(f_job,
        "[%ld] Tech %d finished job %d for %-16s | "
        "%s | pri %d | dur %ds | cost ₹%.2f | wait %.0fs | run %.0fs\n",
        time(NULL), technician_id, job->job_id, job->client_name,
        job->job_type, job->priority, job->duration, cost,
        wait_time, exec_time);
    fflush(f_job);

    /* CSV log */
    fprintf(f_csv,
        "%ld,%d,%s,%s,%d,%d,%d,%.0f,%.2f,%.2f,%.2f,%.2f\n",
        time(NULL), job->job_id, job->client_name, job->job_type,
        job->priority, job->duration, technician_id,
        wait_time, exec_time, cost, cpu_user, cpu_sys);
    fflush(f_csv);
}
