// msg_queue.c
//
// CPSC 351 - Assignment 2 (Part II: POSIX Message Queues)
// -------------------------------------------------------
// Build:
//   gcc -Wall -Wextra -O2 msg_queue.c -o msg_queue -lrt
// Symlinks to match required invocations:
//   ln -sf msg_queue recv
//   ln -sf msg_queue sender
//
// Run (two terminals):
//   ./recv
//   ./sender file.txt
//
// Notes:
// * Spec says receiver queue name "cpsc351queue". POSIX requires a leading '/':
//     MQ_NAME = "/cpsc351queue"
// * Sender must open an existing queue only (no O_CREAT).
// * Receiver blocks in mq_receive() and exits on a 0-byte message.
// * Receiver unlinks the queue on clean termination (so future runs start fresh).

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// ============================================================================
//                                CONFIGURATION
// ============================================================================
#define MQ_NAME   "/cpsc351queue"   // POSIX name (must start with '/')
#define MSG_SIZE  4096              // max message size (bytes)
#define MAX_MSGS  10                // max messages buffered in queue

// ============================================================================
//                            RECEIVER (./recv)
// ----------------------------------------------------------------------------
// Behavior (per spec):
// 1) Create/open message queue MQ_NAME (10 msgs, 4096 bytes each).
// 2) Open "file_recv" for writing (truncate).
// 3) Loop: blocking mq_receive().
//    - If n > 0: write exactly n bytes to file and continue.
//    - If n == 0: close file, mq_close, mq_unlink, exit(0).
// ============================================================================
static int run_receiver(void) {
    // --- Allocate / open the message queue (creator) ---
    struct mq_attr attr = {
        .mq_flags   = 0,        // blocking
        .mq_maxmsg  = MAX_MSGS,
        .mq_msgsize = MSG_SIZE,
        .mq_curmsgs = 0
    };
    
    // Dump any exisitng queue
    mq_unlink(MQ_NAME);  

    // Open with O_CREAT | O_RDONLY to be the creating side and specify attrs.
    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open (receiver)");
        return 1;
    }

    FILE *fp = fopen("file_recv", "wb");
    if (!fp) {
        perror("fopen(file_recv)");
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return 1;
    }

    printf("Receiver ready. Waiting for messages on %s ...\n", MQ_NAME);

    // Fixed-size receive buffer; mq_receive() guarantees n <= MSG_SIZE.
    char    buf[MSG_SIZE];
    unsigned prio = 0;

    for (;;) {
        ssize_t n = mq_receive(mq, buf, sizeof(buf), &prio);
        if (n < 0) {
            // Interrupted by signal? Just retry. Otherwise, fatal.
            if (errno == EINTR) continue;
            perror("mq_receive");
            break;
        }

        if (n == 0) {
            // Terminator per spec (priority 2 recommended by spec, but size==0 is the key)
            printf("Receiver: terminator received (prio=%u). Closing.\n", prio);
            break;
        }

        // Optional: preview to console (safe NUL cap for printing)
        size_t preview = (size_t)n < (MSG_SIZE - 1) ? (size_t)n : (MSG_SIZE - 1);
        buf[preview] = '\0';
        printf("Receiver: got %zd bytes%s\n", n, (prio ? " (prio set)" : ""));

        // Write exactly n bytes, no extra newline
        size_t wrote = fwrite(buf, 1, (size_t)n, fp);
        if (wrote != (size_t)n) {
            perror("fwrite(file_recv)");
            break;
        }
        fflush(fp);
    }

    // Cleanup
    fclose(fp);
    mq_close(mq);
    // As the creator, unlink so repeated runs start with a clean queue
    if (mq_unlink(MQ_NAME) == -1) {
        perror("mq_unlink");
        // Not fatal
    }

    return 0;
}

// ============================================================================
//                            SENDER (./sender <file>)
// ----------------------------------------------------------------------------
/*
Behavior (per spec):
1) Invoked as: ./sender <file name>
2) Open existing message queue MQ_NAME WITHOUT O_CREAT.
   - If not present, error and terminate.
3) Open the input file.
4) Loop:
   (a) Read at most 4096 bytes from the file.
   (b) Send those bytes with priority 1.
   (c) Repeat until EOF.
5) Send an empty (0-byte) message with priority 2 to signal completion.
6) Terminate.
*/
static int run_sender(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ./sender <file>\n");
        return 1;
    }

    const char *path = argv[1];
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        perror("fopen(input)");
        return 1;
    }

    // Open existing queue only (do not create here per spec)
    mqd_t mq = mq_open(MQ_NAME, O_WRONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open (sender) - queue must already exist (start ./recv first)");
        fclose(fp);
        return 1;
    }

    printf("Sender ready. Sending '%s' in chunks up to %d bytes ...\n", path, MSG_SIZE);

    char buf[MSG_SIZE];
    for (;;) {
        size_t n = fread(buf, 1, sizeof(buf), fp);
        if (n == 0) {
            if (ferror(fp)) {
                perror("fread");
            }
            break; // EOF or error
        }

        // Blocking send with priority 1
        if (mq_send(mq, buf, n, 1) == -1) {
            perror("mq_send (data)");
            fclose(fp);
            mq_close(mq);
            return 1;
        }
    }

    // Send terminator: 0-byte with priority 2
    if (mq_send(mq, "", 0, 2) == -1) {
        perror("mq_send (terminator)");
        // continue cleanup anyway
    }

    fclose(fp);
    mq_close(mq);
    printf("Sender done.\n");
    return 0;
}

// ============================================================================
//                                     MAIN
// ----------------------------------------------------------------------------
// We dispatch based on argv[0] so the single binary can act as ./recv or ./sender.
// Create symlinks:
//   ln -sf msg_queue recv
//   ln -sf msg_queue sender
// ============================================================================
static const char *basename_ptr(const char *p) {
    const char *slash = strrchr(p, '/');
    return slash ? (slash + 1) : p;
}

int main(int argc, char **argv) {
    const char *who = basename_ptr(argv[0]);

    if (strcmp(who, "recv") == 0 || strcmp(who, "./recv") == 0) {
        return run_receiver();
    }
    if (strcmp(who, "sender") == 0 || strcmp(who, "./sender") == 0) {
        return run_sender(argc, argv);
    }

    // Friendly fallback if run directly:
    fprintf(stderr,
            "Usage:\n"
            "  ./recv                (create queue, block, write to file_recv, exit on 0-byte msg)\n"
            "  ./sender <file>       (open existing queue, send chunks, send 0-byte terminator)\n");
    return 1;
}

