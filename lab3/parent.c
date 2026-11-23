#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <string.h>

#define SHM_SIZE 4096

int read_line(int fd, char *buf, int max) {
    int i = 0;
    char c = 0;
    while (i < max - 1 && read(fd, &c, 1) > 0) {
        if (c == '\n') break;
        if (c != '\r') buf[i++] = c;
    }
    buf[i] = '\0';
    return (i > 0 || c == '\n');
}

void make_name(char *out, const char *prefix, int id) {
    strcpy(out, prefix);
    int pos = strlen(out);
    char tmp[16];
    int n = 0;
    while (id > 0) {
        tmp[n++] = '0' + (id % 10);
        id /= 10;
    }
    while (n > 0) out[pos++] = tmp[--n];
    out[pos] = '\0';
}

int main(void) {
    char filename[256];
    if (!read_line(0, filename, sizeof(filename)) || filename[0] == '\0') return 1;

    int id = getpid();
    char shm_name[64], s_emp[64], s_ful[64];
    make_name(shm_name, "/shm_", id);
    make_name(s_emp, "/emp_", id);
    make_name(s_ful, "/ful_", id);

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1 || ftruncate(shm_fd, SHM_SIZE) == -1) return 1;

    char *shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if (shm == MAP_FAILED) return 1;

    sem_t *sem_empty = sem_open(s_emp, O_CREAT, 0666, 1);
    sem_t *sem_full = sem_open(s_ful, O_CREAT, 0666, 0);
    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED) return 1;

    pid_t pid = fork();
    if (pid < 0) return 1;

    if (pid == 0) {
        char *args[] = {"./child", filename, shm_name, s_emp, s_ful, NULL};
        execv("./child", args);
        _exit(1);
    }

    char line[SHM_SIZE];
    while (read_line(0, line, sizeof(line))) {
        if (line[0] == '\0') continue;
        sem_wait(sem_empty);
        strcpy(shm, line);
        sem_post(sem_full);
    }

    sem_wait(sem_empty);
    shm[0] = '\0';
    sem_post(sem_full);

    waitpid(pid, NULL, 0);

    sem_close(sem_empty);
    sem_close(sem_full);
    sem_unlink(s_emp);
    sem_unlink(s_ful);
    munmap(shm, SHM_SIZE);
    shm_unlink(shm_name);
    return 0;
}
