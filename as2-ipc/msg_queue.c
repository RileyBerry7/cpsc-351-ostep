// msg_queue.c

// Use: open(), read(), write() syscalls

// Create Msg Queue: mq_open(queue_id);
// 	-> returns: mqd_t 'msg queue descriptor' used to refer msg queue
// Queue Identifier:  '/queue_name'
// Produce: mq_send();
// Consume: mq_receive();
// mq_close(queue_id);
// mq_unlink(queue_id);
// mq_getattr();
// mq_notify();  - Allows the calling process to register or unregister for delivery of an asynchronous notification 
//		   when a new message arrives on the empty message queue referred to by the message queue descriptor mqdes.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MQ_NAME "/cpsc351queue"

// Message Struct
typedef struct {
	char msg[80];
} Message;

// Signal Handler Function
void handler(int sig);


// Queue Attributes
struct mq_attr attributes = {
	.mq_flags = 0, 
	.mq_maxmsg = 10,
	.mq_curmsgs = 0,
	.mq_msgsize = sizeof(Message) // 4096 bytes
};

/**** MAIN ************************+****/
int main(int argc, char *argv[]) {
	
	int    ret;
	int    oflags;
	mode_t mode;
	
	// ---------- RECIEVER ----------
	if (!strcmp(argv[0], "./rcvr")) {

		printf("Ready to receive.\n");
		oflags = O_CREAT | O_RDONLY;
		mode   = S_IRUSR | S_IWUSR;

		// 2. Create/Open Queue
		mqd_t mq = mq_open(MQ_NAME, oflags, mode, &attributes);
		if (mq == (mqd_t)-1) {
			perror("mq_open");
			exit(EXIT_FAILURE);
		}
		
		// 3. Block until msg queue is non-empty	
		getchar();
		struct sigevent not;
		
		// Setup signal handler
		struct sigaction sa;
		sa.sa_handler = handler;
		sa.sa_flags   = 0;
		sigemptyset(&sa.sa_mas);
		sigaction(SIGUSR1, &sa, NULL);

		// Register for notifications
		struct sigevent sev;
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo  = SIGUSR1;

		ret = mq_notify(mq, &sev);
		if (ret == -1) {
			perror("mq_notify");
			exit(EXIT_FAILURE)
		}
		
		// Receiver will wait for a notification
		printf("Waiting for message.")
		while (1)
			pause();


	// ---------- SENDER ----------
	} else if (!strcmp(argv[0], "./sndr")) {

		printf("Ready to send.\n");
		oflags = O_CREAT | O_WRONLY | O_NONBLOCK;
		mode   = S_IRUSR | S_IWUSR;

		// Create/Open Queue
		mqd_t mq = mq_open(MQ_NAME, oflags, mode, &attributes);
		if (mq == (mqd_t)-1) {
			perror("mq_open");
			exit(1);
		}

		// Send Message
		Message outgoing;
		strcpy(outgoing.msg, "lol");
		mq_send(mq, (char *)&outgoing, sizeof(outgoing), 1);

		mq_close(mq);
		mq_unlink(MQ_NAME); //

	}

	printf("Exiting {}\n", argv[0]);
	return 0;
}


/****** SIGNAL HANDLER **###**********************+****/
void handler(int sig) {
	char *incoming;
	ssize_t n; // bytes received

	// Re-subscribe for notifcation
	struct sigevent sev {
		.sigev_notify = SIGEV_SIGNAL;
		.sigev_signo  = SIGUSR1;
	}

	ret = mq_notify(mq, &sev);
	incoming = malloc(sizeof(Message));

	// Receive Message
	n   = mq_receive(mq, buf, attr.mq_msgsize, NULL);

// 4. Recieve Message & Check it's length
		ret = mq_receive(mq, (char *)&incoming, sizeof(incoming), NULL);
		if (ret == -1) {
		    perror("mq_receive");
		    exit(EXIT_FAILURE);
		}

		// Write message to file

		// Exit on empty message
		
		printf("Recieved: %s\n", incoming.msg);
		
		mq_close(mq);
		
	// Handle Message
	if (n >= 0) {
		print("Message Received.\n");
	}

	free(buf);

	printf("Exiting Signal Handler.\n")
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
