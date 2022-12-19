#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "apriori.h"

#define DEFAULT_MIN_SUPPORT 0.005
#define DEFAULT_MIN_CONFIDENCE 0.6

typedef struct {
    struct timeval startTime;
    struct timeval endTime;
} Timer;

void startTime(Timer *timer) { gettimeofday(&(timer->startTime), NULL); }

void stopTime(Timer *timer) { gettimeofday(&(timer->endTime), NULL); }

float elapsedTime(Timer timer) {
    return ((float)((timer.endTime.tv_sec - timer.startTime.tv_sec) +
                    (timer.endTime.tv_usec - timer.startTime.tv_usec) / 1.0e6));
}

int main(int argc, char *argv[]) {
    // Read input arguments
    if (argc != 2 && argc != 4) {
        fprintf(stderr,
                "Please provide an input csv file, a minimum support and a "
                "minimum confidence.\n");
        exit(EXIT_FAILURE);
    }

    // Use some defaults
    float minSupport = DEFAULT_MIN_SUPPORT;
    float minConfidence = DEFAULT_MIN_CONFIDENCE;
    if (argc == 4) {
        minSupport = atof(argv[2]);
        minConfidence = atof(argv[3]);
    } else {
        fprintf(stderr,
                "No minimum support and confidence provided; using defaults %.3lf "
                "and %.1lf\n\n",
                minSupport, minConfidence);
    }
    Timer timer;
    startTime(&timer);
    apriori(argv[1], minSupport, minConfidence);

    stopTime(&timer);
    printf("\nExecution took %lf sec.\n", elapsedTime(timer));

    return EXIT_SUCCESS;
}
