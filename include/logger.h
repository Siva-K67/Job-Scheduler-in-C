#ifndef LOGGER_H
#define LOGGER_H

#include "jobs.h"

void init_logger(void);
void log_job_completion(const Job *job,
                        int technician_id,
                        float cost,
                        double wait_time,
                        double exec_time,
                        double cpu_user,
                        double cpu_sys);

#endif
