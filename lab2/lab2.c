#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>

#define N (1 << 20)

int *arr_seq, *arr_par;
int T = 4;
pthread_barrier_t bar;

int my_atoi(char *s) {
    int r = 0;
    while (*s >= '0' && *s <= '9') r = r * 10 + (*s++ - '0');
    return r;
}

void print_str(char *s) {
    int len = 0;
    while (s[len]) len++;
    write(1, s, len);
}

void print_num(long n) {
    char b[32];
    int i = 30;
    b[31] = '\n';
    if (n == 0) b[i--] = '0';
    while (n > 0) {
        b[i--] = (n % 10) + '0';
        n /= 10;
    }
    write(1, &b[i + 1], 31 - i);
}

long time_ms() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void *worker(void *arg) {
    long id = (long)arg;
    int chunk = (N + T - 1) / T;
    int start = id * chunk;
    int end = start + chunk > N ? N : start + chunk;

    for (int k = 2; k <= N; k <<= 1) {
        for (int j = k >> 1; j > 0; j >>= 1) {
            for (int i = start; i < end; i++) {
                int l = i ^ j;
                if (l > i) {
                    int dir = ((i & k) == 0);
                    if ((arr_par[i] > arr_par[l]) == dir) {
                        int t = arr_par[i];
                        arr_par[i] = arr_par[l];
                        arr_par[l] = t;
                    }
                }
            }
            pthread_barrier_wait(&bar);
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1) T = my_atoi(argv[1]);

    arr_seq = mmap(0, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    arr_par = mmap(0, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    pthread_t *th = mmap(0, T * sizeof(pthread_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    unsigned int seed = 42;
    for (int i = 0; i < N; i++) {
        seed = seed * 1103515245 + 12345;
        arr_seq[i] = arr_par[i] = (seed >> 16) & 0x7FFF;
    }

    long t0 = time_ms();
    for (int k = 2; k <= N; k <<= 1)
        for (int j = k >> 1; j > 0; j >>= 1)
            for (int i = 0; i < N; i++) {
                int l = i ^ j;
                if (l > i && (arr_seq[i] > arr_seq[l]) == ((i & k) == 0)) {
                    int t = arr_seq[i];
                    arr_seq[i] = arr_seq[l];
                    arr_seq[l] = t;
                }
            }
    long t_seq = time_ms() - t0;

    pthread_barrier_init(&bar, 0, T);
    long t1 = time_ms();
    for (long i = 0; i < T; i++) pthread_create(&th[i], 0, worker, (void *)i);
    for (long i = 0; i < T; i++) pthread_join(th[i], 0);
    long t_par = time_ms() - t1;
    pthread_barrier_destroy(&bar);

    print_str("Threads: "); print_num(T);
    print_str("Seq time (ms): "); print_num(t_seq);
    print_str("Par time (ms): "); print_num(t_par);

    return 0;
}
