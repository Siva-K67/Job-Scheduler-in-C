#ifndef JOBS_H
#define JOBS_H

#include <time.h>

typedef struct {
    int job_id;
    char client_name[100];
    char job_type[50];
    int duration;        // in seconds
    int priority;
    float estimated_cost;
    time_t request_time;
    time_t  arrival_time;
} Job;

#endif