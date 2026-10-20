#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

static int read_line(int fd, char *buf, int max_len) {
    int i = 0;
    char c = 0;
    while (i < max_len - 1 && read(fd, &c, 1) > 0) {
        if (c == '\n') break;
        if (c != '\r') buf[i++] = c;
    }
    buf[i] = '\0';
    return (i > 0 || c == '\n');
}

static int to_str(long val, char *buf) {
    char tmp[32];
    int len = 0, i = 0;
    if (val == 0) {
        buf[0] = '0';
        return 1;
    }
    if (val < 0) {
        buf[len++] = '-';
        val = -val;
    }
    while (val > 0) {
        tmp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    while (i > 0) {
        buf[len++] = tmp[--i];
    }
    return len;
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return 1;

    char line[1024];
    while (read_line(0, line, sizeof(line))) {
        char *ptr = line;
        long sum = 0;
        int found = 0;

        while (*ptr) {
            char *next;
            long val = strtol(ptr, &next, 10);
            if (ptr == next) {
                ptr++;
            } else {
                sum += val;
                found = 1;
                ptr = next;
            }
        }

        if (found) {
            char out[32];
            int len = to_str(sum, out);
            out[len++] = '\n';
            write(fd, out, len);
        }
    }

    close(fd);
    return 0;
}
