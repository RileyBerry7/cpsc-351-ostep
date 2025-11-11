// pipefile.c
//
// CPSC 351 – Assignment 2 (Part III: POSIX Pipes)
// -------------------------------------------------------
// Build:
//   gcc -Wall -Wextra -O2 -std=c17 -o pipefile pipefile.c
//   or
//   ./build_pipefile.sh
//
// Run:
//   ./pipefile <file>
//
// Requirement Notes:
//   - Single program. Parent -> Child pipe flow.
//   - Parent reads the input file.
//   - Parent writes to pipe.
//   - Child reads from pipe. 
//   - Child writes to "file_recv", then exits on EOF.
//   - Parent waits for the child before terminating.

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>     
#include <sys/types.h>  
#include <sys/wait.h>  
#include <unistd.h>    

// ============================================================================
//                                CONFIGURATION
// ============================================================================
enum { BUFSZ = 4096 };  // required max transfer size


// ============================================================================
//                         HELPER write_all()
// ----------------------------------------------------------------------------
// This function keeps calling write() until we've pushed the whole buffer 
// or hit an error. Retries on 'EINTR' signal.
// // ============================================================================
static ssize_t write_all(int fd, const void *buf, size_t n) {
    const char *p = (const char *)buf;
    size_t left = n;

    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;   // try again if interrupted
            return -1;                      // error
        }
        if (w == 0) {
            // This shouldn't happen with pipes -> ERROR
            errno = EIO;
            return -1;
        }
        p    += w;
        left -= (size_t)w;
    }
    return (ssize_t)n;
}


// ============================================================================
//                                     MAIN
// ----------------------------------------------------------------------------
// Behavior:
//   1. Create pipe()
//   2. fork()
//   3. Parent:
//        - close read end
//        - open <source_file>
//        - read 4096B, write to pipe (loop until EOF)
//        - close write end (signals EOF to child)
//        - wait for child
//   4. Child:
//        - close write end.
//        - open "file_recv".
//        - read from pipe until read returns 0.
//        - write each chunk to file
//        - close pipe, exit
// ============================================================================
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    const char *in_path = argv[1];
    int fds[2];  // fds[0] = read end, fds[1] = write end

    // --- Step 1: create the pipe ---
    if (pipe(fds) == -1) {
        perror("pipe");
        return 1;
    }

    // --- Step 2: fork a child ---
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        // best effort cleanup if fork failed
        close(fds[0]);
        close(fds[1]);
        return 1;
    }

    // ========================================================================
    // CHILD PROCESS
    // ------------------------------------------------------------------------
    // - Close the write end (we only read).
    // - Open output "file_recv".
    // - Loop: read from pipe, write to file (handle partial writes).
    // - Exit when read() returns 0.
    // ========================================================================
    if (pid == 0) {
        // Close the end we don't need; otherwise read() might never see EOF.
        if (close(fds[1]) == -1) {
            perror("child: close(write-end)");
            // continue anyway; we still try to read
        }

        int out_fd = open("file_recv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd == -1) {
            perror("open(file_recv)");
            close(fds[0]);
            _exit(1);
        }

        char buf[BUFSZ];
        for (;;) {
            ssize_t r = read(fds[0], buf, sizeof buf);
            if (r < 0) {
                if (errno == EINTR) continue; // interrupted? try again
                perror("read(pipe)");
                close(out_fd);
                close(fds[0]);
                _exit(1);
            }
            if (r == 0) {
                // Parent closed its write end -> EOF
                break;
            }
            if (write_all(out_fd, buf, (size_t)r) == -1) {
                perror("write(file_recv)");
                close(out_fd);
                close(fds[0]);
                _exit(1);
            }
        }

        if (close(out_fd) == -1) {
            perror("close(file_recv)");
        }
        if (close(fds[0]) == -1) {
            perror("child: close(read-end)");
        }
        _exit(0);
    }

    // ========================================================================
    // PARENT PROCESS
    // ------------------------------------------------------------------------
    // - Close the read end (we only write).
    // - Open the input file (argv[1]) for reading.
    // - Loop: read 4096B -> write_all() to pipe.
    // - Close write end to send EOF to child.
    // - waitpid() for the child and report if it failed.
    // ========================================================================
    // Close the end we don't use on the parent side.
    if (close(fds[0]) == -1) {
        perror("parent: close(read-end)");
        // not fatal
    }

    int in_fd = open(in_path, O_RDONLY);
    if (in_fd == -1) {
        fprintf(stderr, "open(%s): %s\n", in_path, strerror(errno));

        // Close write end to let child unblock/exit, then wait.
        close(fds[1]);
        int status;
        (void)waitpid(pid, &status, 0);
        return 1;
    }

    char buf[BUFSZ];
    for (;;) {
        ssize_t r = read(in_fd, buf, sizeof buf);
        if (r < 0) {
            if (errno == EINTR) continue; // interrupted? just retry
            fprintf(stderr, "read(%s): %s\n", in_path, strerror(errno));
            close(in_fd);
            close(fds[1]); // signal EOF to child so it can finish
            int status;
            (void)waitpid(pid, &status, 0);
            return 1;
        }
        if (r == 0) {
            // EOF
            break;
        }
        if (write_all(fds[1], buf, (size_t)r) == -1) {
            // If the child died early, this can be EPIPE.
            perror("write(pipe)");
            close(in_fd);
            close(fds[1]);
            int status;
            (void)waitpid(pid, &status, 0);
            return 1;
        }
    }

    if (close(in_fd) == -1) {
        perror("close(input)");
        // keep going
    }

    // Very important: closing parent's write end tells the child "no more data".
    if (close(fds[1]) == -1) {
        perror("parent: close(write-end)");
    }

    // Wait for the child to finish writing out.
    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    // Quick sanity check — not required, but nice to have.
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        // printf("Done. Child exited cleanly.\n");
        return 0;
    } else {
        fprintf(stderr, "child exited abnormally\n");
        return 1;
    }
}

// el fin
