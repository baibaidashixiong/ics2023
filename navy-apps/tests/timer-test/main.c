#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

int main() {
    struct timeval init;
    struct timeval now;
    printf("timer zqz\n");
    assert(gettimeofday(&init, NULL) == 0);
    time_t init_sec = init.tv_sec;
    suseconds_t init_usec = init.tv_usec;

    size_t times = 1;

    while (1) {
        assert(gettimeofday(&now, NULL) == 0);
        time_t now_sec = now.tv_sec;
        suseconds_t now_usec = now.tv_usec;
        uint64_t time_gap = (now_sec - init_sec) * 1000000 + (now_usec - init_usec); // unit: us
        if (time_gap > 500000 * times) {
            printf("Half a second passed, %u time(s)\n", times);
            times++;
        }
    }
}