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

// Signal Event
struct sigevent sev = {
	.sigev_notify = SIGEV_SIGNAL,
	.sigev_signo  = SIGUSR1,
};


/**** MAIN ************************+****/
int main(int argc, char *argv[]) {
	

	// Signal Action
	struct sigaction sa = {
		.sa_handler = handler,
		.sa_flags   = 0,
		sigemptyset(&sa.sa_mask),
		sigaction(SIGUSR1, &sa, NULL)
	};

	int    ret;
	int    oflags;
	mode_t mode;
	mqd_t  mq;
	
	// ---------- RECIEVER ----------
	if (!strcmp(argv[0], "./rcvr")) {

		printf("Ready to receive.\n");
		oflags = O_CREAT | O_RDONLY;
		mode   = S_IRUSR | S_IWUSR;

		// 2. Create/Open Queue
		mq = mq_open(MQ_NAME, oflags, mode, &attributes);
		if (mq == (mqd_t)-1) {
			perror("mq_open");
			exit(EXIT_FAILURE);
		}
		
		// Register for notifications
		ret = mq_notify(mq, &sev);
		if (ret == -1) {
			perror("mq_notify");
			exit(EXIT_FAILURE);
		}
		
		// 3. Receiver will wait for a notification
		printf("Waiting for notification...\n");
		while (1)
			pause();
		
		mq_close(mq);
		mq_unlink(MQ_NAME); //

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
		mq_send(mq, (char *)&outgoing, sizeof(outgoing), 1);

		mq_close(mq);

	}

	printf("Exiting %s\n", argv[0]);
	return 0;
}


/****** SIGNAL HANDLER **###**********************+****/
void handler(int sig) {

	Message  incoming;
	int      ret;
	mqd_t    mq;
	int      oflags = O_CREAT | O_RDONLY;
	mode_t   mode   = S_IRUSR | S_IWUSR;

	// Open Queue
	mq = mq_open(MQ_NAME, oflags, mode, &attributes);

	// Re-register for notifcation
	ret = mq_notify(mq, &sev);

	// 4. Recieve Message & Check it's length
	ret = mq_receive(mq, (char *)&incoming, sizeof(incoming), NULL);
	if (ret == -1) {
	    perror("mq_receive");
	    exit(EXIT_FAILURE);
	}

	// Write message to file

	// Exit on empty message
		
		
	// Handle Message
	if (ret >= 0) {
		printf("Recieved: %s\n", incoming.msg);
	}

	mq_close(mq);
	printf("Exiting Signal Handler.\n");
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
