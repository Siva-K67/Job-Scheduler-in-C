#ifndef TECHNICIAN_H
#define TECHNICIAN_H

#include <time.h>

typedef struct {
    int tech_id;
    int is_available;
    pthread_t thread_id;
} Technician;

#endif