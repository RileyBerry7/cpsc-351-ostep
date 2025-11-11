// sender.c
//
// CPSC 351 – Assignment 2 (Part I: POSIX Shared Memory)
// -------------------------------------------------------
// Build:
//   gcc -Wall -Wextra -O2 -std=c17 -o sender sender.c
//   or
//   ./build_p1.sh
//
// Run:
//   ./sender <file> <receiver_pid>
//
// Notes:
// - Creates /cpsc351sharedmem with 0600 perms.
// - Sizes SHM to file size, copies bytes in.
// - Sends SIGUSR1 to receiver PID.

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

#define SHM_NAME "/cpsc351sharedmem"   // must match receiver

static off_t get_file_size(int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) return -1;
    return st.st_size;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file> <receiver_pid>\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    int recv_pid = atoi(argv[2]);
    if (recv_pid <= 0) {
        fprintf(stderr, "Invalid receiver PID.\n");
        return 1;
    }

    // open input file
    int in_fd = open(path, O_RDONLY);
    if (in_fd == -1) {
        perror("open(input)");
        return 1;
    }

    off_t fsize = get_file_size(in_fd);
    if (fsize < 0) {
        perror("fstat(input)");
        close(in_fd);
        return 1;
    }

    // create SHM with 0600 perms as required
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shm_open");
        close(in_fd);
        return 1;
    }

    // set size to file size
    if (ftruncate(shm_fd, fsize) == -1) {
        perror("ftruncate");
        close(shm_fd);
        close(in_fd);
        shm_unlink(SHM_NAME); // best-effort cleanup
        return 1;
    }

    // map SHM for writing; size could be zero, handle that
    void *shm_ptr = NULL;
    if (fsize > 0) {
        shm_ptr = mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shm_ptr == MAP_FAILED) {
            perror("mmap");
            close(shm_fd);
            close(in_fd);
            shm_unlink(SHM_NAME);
            return 1;
        }
    }

    // copy file -> SHM in chunks (simple and safe)
    if (fsize > 0) {
        const size_t CHUNK = 4096;
        char buf[4096];
        ssize_t r;
        off_t offset = 0;

        while ((r = read(in_fd, buf, CHUNK)) > 0) {
            // memcpy into mapped region
            memcpy((char *)shm_ptr + offset, buf, (size_t)r);
            offset += r;
        }
        if (r == -1) {
            perror("read(input)");
            // fall through to cleanup; receiver will still try to read whatever we wrote
        }
        // msync is optional here; mapping is MAP_SHARED and we're about to signal
        // msync(shm_ptr, fsize, MS_SYNC);
    }

    // done with file + shm fd (keep object; receiver will unlink)
    if (fsize > 0 && shm_ptr && shm_ptr != MAP_FAILED) munmap(shm_ptr, fsize);
    close(shm_fd);
    close(in_fd);

    // wake the receiver
    if (kill(recv_pid, SIGUSR1) == -1) {
        perror("kill(SIGUSR1)");
        // don't unlink; receiver might still be started later for grading consistency
        return 1;
    }

    return 0;
}

