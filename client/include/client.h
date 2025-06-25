
#ifndef CLIENT_H
#define CLIENT_H

#define MAX_STR 64

typedef struct {
    char client_name[MAX_STR];
    char job_type[MAX_STR];
    int priority;
    int duration;
} Job;

void clear_screen();
void print_banner();
void get_job_input(Job *job);

#endif

