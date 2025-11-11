// recv.c
//
// CPSC 351 – Assignment 2 (Part I: POSIX Shared Memory)
// -------------------------------------------------------
// Build:
//   gcc -Wall -Wextra -O2 -std=c17 -o recv recv.c
//   or
//   ./build_p1.sh
//
// Run:
//   ./recv
//
// Notes:
// - Waits for SIGUSR1.
// - On signal: reads /cpsc351sharedmem into file_recv, then deallocates SHM and exits.

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

// ============================================================================
//                                CONFIGURATION
// ============================================================================
#define SHM_NAME "/cpsc351sharedmem"   // must match sender, leading '/' required

// ============================================================================
//                               FILE RECEIVER: recvFile()
// ----------------------------------------------------------------------------
// Called on SIGUSR1. Do the whole job, then exit.
// Yes, this does a lot inside a signal handler. That's the assignment.
// ============================================================================
static void recvFile(int sigNum)
{
    (void)sigNum;

    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0);
    if (shm_fd == -1) {
        // exact message per spec
        fprintf(stderr, "Missing shared memory segment!\n");
        _exit(1);
    }

    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        perror("fstat");
        close(shm_fd);
        _exit(1);
    }

    // map SHM read-only; size is whatever the sender set via ftruncate
    void *shm_ptr = NULL;
    if (st.st_size > 0) {
        shm_ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
        if (shm_ptr == MAP_FAILED) {
            perror("mmap");
            close(shm_fd);
            _exit(1);
        }
    }

    // open destination file (truncate); use 0666 like the spec examples
    int out_fd = open("file_recv", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (out_fd == -1) {
        perror("open(file_recv)");
        if (st.st_size > 0 && shm_ptr && shm_ptr != MAP_FAILED) munmap(shm_ptr, st.st_size);
        close(shm_fd);
        _exit(1);
    }

    // write all bytes in one go (okay for this assignment)
    if (st.st_size > 0) {
        ssize_t w = write(out_fd, shm_ptr, st.st_size);
        if (w == -1 || w != st.st_size) {
            perror("write");
            // still clean up and exit
        }
    }

    // cleanup: file, mapping, fd, and unlink SHM (receiver deallocates)
    close(out_fd);
    if (st.st_size > 0 && shm_ptr && shm_ptr != MAP_FAILED) munmap(shm_ptr, st.st_size);
    close(shm_fd);
    shm_unlink(SHM_NAME);

    // success
    _exit(0);
}

// ============================================================================
//                                      MAIN
// ----------------------------------------------------------------------------
int main(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = recvFile;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;   // fine for this

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Receiver PID: %d\n", getpid());
    printf("Waiting for SIGUSR1 from sender...\n");

    // sleep forever until signal arrives; handler exits the process
    for (;;) pause();
}

