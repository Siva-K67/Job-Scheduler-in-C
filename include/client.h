#ifndef CLIENT_H
#define CLIENT_H

#include "jobs.h"  // ✅ Job struct comes from here

void clear_screen();
void print_banner();
void get_job_input(Job *job);

#endif
