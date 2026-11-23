#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdlib.h>

#define SHM_SIZE 4096

int to_str(long val, char *buf) {
    char tmp[32];
    int len = 0, i = 0;
    if (val == 0) { buf[0] = '0'; return 1; }
    if (val < 0) {buf[len++] = '-'; val = -val;}
    while (val > 0) {tmp[i++] = '0' + (val%10); val /= 10;}
    while (i > 0) buf[len++] = tmp[--i];
    return len;
}

int main(int argc, char *argv[]) {
    if (argc < 5) return 1;

    int out_fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int shm_fd = shm_open(argv[2], O_RDWR, 0666);
    if (out_fd < 0 || shm_fd < 0) return 1;

    char *shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if (shm == MAP_FAILED) return 1;

    sem_t *sem_empty = sem_open(argv[3], 0);
    sem_t *sem_full = sem_open(argv[4], 0);
    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED) return 1;

    while (1) {
        sem_wait(sem_full);
        if (shm[0] == '\0') {
            sem_post(sem_empty);
            break;
        }

        char *ptr = shm;
        long sum = 0;
        int found = 0;
        while (*ptr) {
            char *next;
            long val = strtol(ptr, &next, 10);
            if (ptr == next) ptr++;
            else { sum += val; found = 1; ptr = next; }
        }

        if (found) {
            char out[32];
            int len = to_str(sum, out);
            out[len++] = '\n';
            write(out_fd, out, len);
        }

        sem_post(sem_empty);
    }

    sem_close(sem_empty);
    sem_close(sem_full);
    munmap(shm, SHM_SIZE);
    close(out_fd);
    return 0;
}
