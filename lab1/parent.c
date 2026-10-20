#include <unistd.h>
#include <sys/wait.h>

static int read_line(int fd, char *buf, int max_len) {
    int i = 0;
    char c = 0;
    while (i < max_len - 1 && read(fd, &c, 1) > 0) {
        if (c == '\n') break;
        if (c != '\r') buf[i++] = c;
    }
    buf[i] = '\0';
    return (i > 0);
}

int main(void) {
    char filename[256];
    if (!read_line(0, filename, sizeof(filename))) {
        return 1;
    }

    int p1[2], p2[2];
    if (pipe(p1) == -1 || pipe(p2) == -1) {
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        return 1;
    }

    if (pid == 0) {
        dup2(p1[0], 0);
        dup2(p2[1], 2);

        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);

        char *args[] = {"./child", filename, NULL};
        execv("./child", args);
        _exit(1);
    }

    close(p1[0]);
    close(p2[1]);

    char buf[512];
    ssize_t bytes;
    while ((bytes = read(0, buf, sizeof(buf))) > 0) {
        write(p1[1], buf, bytes);
    }

    close(p1[1]);
    close(p2[0]);

    waitpid(pid, NULL, 0);
    return 0;
}
