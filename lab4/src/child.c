#include "common.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

void reverse_line(char *line, size_t length) {
    if (length < 2) {
        return;
    }

    size_t left = 0;
    size_t right = length - 1;

    while (left < right) {
        char temp = line[left];
        line[left] = line[right];
        line[right] = temp;
        left++;
        right--;
    }
}

int process_line(int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("child: fstat");
        return -1;
    }

    if (st.st_size <= 0) {
        fprintf(stderr, "child: empty mailbox after SIGUSR1\n");
        return -1;
    }

    size_t length = (size_t)st.st_size;
    char *line = mmap(
        NULL,
        length,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );
    if (line == MAP_FAILED) {
        perror("child: mmap");
        return -1;
    }

    size_t text_length = length;
    if (line[length - 1] == '\n') {
        --text_length;
    }

    reverse_line(line, text_length);

    int result = 0;
    if (write_all(STDOUT_FILENO, line, length) == -1) {
        perror("child: write");
        result = -1;
    }
    
    if (munmap(line, length) == -1) {
        perror("child: munmap");
        result = -1;
    }

    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: child <mailbox-path>\n");
        return EXIT_FAILURE;
    }

    pid_t parent_pid = getppid();
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("child: open mailbox");
        return EXIT_FAILURE;
    }

    sigset_t signals;
    int status = EXIT_FAILURE;

    if (sigemptyset(&signals) == -1 ||
        sigaddset(&signals, SIGUSR1) == -1 ||
        sigaddset(&signals, SIGTERM) == -1) {
        perror("child: signal set");
        goto cleanup;
    }

    if (fprintf(stderr, "child PID=%ld, parent PID=%ld\n",
                (long)getpid(), (long)parent_pid) < 0) {
        perror("child: diagnostic output");
        goto cleanup;
    }

    while (1) {
        int signal_number;

        do {
            signal_number = sigwaitinfo(&signals, NULL);
        } while (signal_number == -1 && errno == EINTR);

        if (signal_number == -1) {
            perror("child: sigwaitinfo");
            break;
        }

        if (signal_number == SIGTERM) {
            status = EXIT_SUCCESS;
            break;
        }

        if (process_line(fd) == -1) {
            break;
        }

        if (kill(parent_pid, SIGUSR2) == -1) {
            perror("child: acknowledge");
            break;
        }
    }
    
cleanup:
    if (close(fd) == -1) {
        perror("child: close mailbox");
        status = EXIT_FAILURE;
    }
    if (close(STDOUT_FILENO) == -1) {
        perror("child: close stdout");
        status = EXIT_FAILURE;
    }

    return status;
}