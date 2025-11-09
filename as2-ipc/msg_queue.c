// msg_queue.c

// Use: open(), read(), write() syscalls

// Create Msg Queue: mq_open(queue_id);
//  -> returns: mqd_t 'msg queue descriptor' used to refer msg queue
// Queue Identifier:  '/queue_name'
// Produce: mq_send();
// Consume: mq_receive();
// mq_close(queue_id);
// mq_unlink(queue_id);
// mq_getattr();
// mq_notify();  - Allows the calling process to register or unregister for delivery of an asynchronous notification 
//                 when a new message arrives on the empty message queue referred to by the message queue descriptor mqdes.
//
// A 'message queue descriptor' is a reference to an open message queue descriptio.
// After a fork(), children inherity copies of mqd_t of the parent.
// Copied mqd_t will share the flags (mq_flags) that are associated with that mqd_t.

//   Linking
//       Programs using the POSIX message queue API must be compiled with
//       cc -lrt to link against the real-time library, librt.

#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MQ_NAME "/cpsc351queue"

static volatile sig_atomic_t notified = 0;   // FIX: will be set to 1 in handler

// Message Struct
typedef struct {
    char msg[80];
} Message;

// Queue Attributes
struct mq_attr attributes = {
    .mq_flags   = 0, 
    .mq_maxmsg  = 10,
    .mq_curmsgs = 0,
    .mq_msgsize = sizeof(Message) // 4096 bytes
};

// Signal Event
struct sigevent sev = {
    .sigev_notify = SIGEV_SIGNAL,
    .sigev_signo  = SIGUSR1,
    .sigev_value.sival_int = 0
};

/****** SIGNAL HANDLER *******************************************/
static void handler(int sig){
    (void)sig;
    notified = 1;           // FIX: set (not clear) the notification flag
}

/**** MAIN ********************************************************/
int main(int argc, char *argv[]) {
    int    ret;
    int    oflags;
    mode_t mode;
    mqd_t  mq;

    // Signal Action
    sigset_t block, prev, waitmask;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);

    // Block SIGUSR1 before arming mq_notify (canonical pattern)
    if (sigprocmask(SIG_BLOCK, &block, &prev) == -1) { perror("sigprocmask"); exit(1); }

    // FIX: install SIGUSR1 handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // SA_RESTART optional; not needed for sigsuspend
    if (sigaction(SIGUSR1, &sa, NULL) == -1) { perror("sigaction"); exit(1); }

    // ---------- RECIEVER ----------
    if (!strcmp(argv[0], "./rcvr")) {

        printf("Ready to receive.\n");
        oflags = O_CREAT | O_RDONLY | O_NONBLOCK;
        mode   = S_IRUSR | S_IWUSR;

        // Create/Open Queue
        mq = mq_open(MQ_NAME, oflags, mode, &attributes);
        if (mq == (mqd_t)-1) {
            perror("mq_open");
            exit(1);
        }

        // Notification Loop
        for(;;){
            // Register for notifications (one-shot). If someone else armed it, EBUSY is fine.
            if (mq_notify(mq, &sev) == -1 && errno != EBUSY) { perror("mq_notify"); break; }

            // Receive Loop: drain anything currently in the queue
            for(;;) {
                Message incoming;
                ssize_t n = mq_receive(mq, (char*)&incoming, sizeof(incoming), NULL);
                if (n >= 0) { printf("Received: %s\n", incoming.msg); continue; }
                if (errno == EAGAIN) break;   // nothing left right now
                perror("mq_receive");
                goto out;
            }

            // Wait for next notification
            notified = 0;

            // FIX: wait mask = previous mask but with SIGUSR1 unblocked during sigsuspend()
            waitmask = prev;
            sigdelset(&waitmask, SIGUSR1);

            while (!notified) {
                errno = 0;
                sigsuspend(&waitmask);        // wakes with EINTR on SIGUSR1
                if (errno != EINTR && errno != 0) { perror("sigsuspend"); }
            }
        }
    out:
        // Restore original mask and close
        if (sigprocmask(SIG_SETMASK, &prev, NULL) == -1) { perror("sigprocmask restore"); }
        mq_close(mq);

    // ---------- SENDER ----------
    } else if (!strcmp(argv[0], "./sndr")) {

        printf("Ready to send.\n");
        oflags = O_CREAT | O_WRONLY | O_NONBLOCK;
        mode   = S_IRUSR | S_IWUSR;

        // Create/Open Queue
        mq = mq_open(MQ_NAME, oflags, mode, &attributes);
        if (mq == (mqd_t)-1) {
            perror("mq_open");
            exit(1);
        }

        // Send Message
        Message outgoing;
        strcpy(outgoing.msg, "lol");
        if (mq_send(mq, (char *)&outgoing, sizeof(outgoing), 1) == -1) {
            perror("mq_send");
        } else {
            printf("Sent.\n");
        }

        mq_close(mq);
    }

    printf("Exiting %s\n", argv[0]);
    return 0;
}

// ___Notes From The Man Page___
// The  fields  of the struct mq_attr pointed to attr specify the maximum number of messages and the maximum size of messages
//        that the queue will allow.  This structure is defined as follows:
//
//            struct mq_attr {
//                long mq_flags;       /* Flags (ignored for mq_open()) */
//                long mq_maxmsg;      /* Max. # of messages on queue */
//                long mq_msgsize;     /* Max. message size (bytes) */
//                long mq_curmsgs;     /* # of messages currently in queue
//                                        (ignored for mq_open()) */
//            };

