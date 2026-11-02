#include <errno.h>
#include <unistd.h>
#include <stdio.h>

int write_all(int fd, const char *data, size_t length) {
    size_t total = 0;

    while (total < length) {
        ssize_t written = write(fd, data + total, length - total);
        
        if (written == -1) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        if (written == 0) {
            errno = EIO;
            return -1;
        }

        total += (size_t)written;
    }

    return 0;
}

ssize_t read_line_retry(FILE *stream, char **line, size_t *capacity) {
    while (1) {
        errno = 0;
        ssize_t length = getline(line, capacity, stream);

        if (length >= 0) {
            return length;
        }

        if (errno != EINTR) {
            return -1;
        }

        clearerr(stream);
    }
}