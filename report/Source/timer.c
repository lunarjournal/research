#include <stdio.h>
#include <sys/time.h>

int main () {

    // before service outage
    struct timeval begin, end;
    gettimeofday(&begin, 0);
    
    
    // after service outage
    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds*1e-6;

    return 0;
}