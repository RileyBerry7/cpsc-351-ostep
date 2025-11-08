// msg_queue.c

// Use: open(), read(), write() syscalls

// Create Msg Queue: mqopen(queue_id);
// 	-> returns: mqd_t 'msg queue descriptor' used to refer msg queue

// Queue Identifier:  '/queue_name'

// Produce: mq_send();
// Consume: mq_receive();

// mq_close(queue_id);
// mq_unlink(queue_id);

// mq_getattr();
// mq_notify();
// A 'message queue descripotr' is a reference to an open message queue descriptio.
// After a fork(), children inherity copies of mqd_t of the parent.
// Copied mqd_t will share the flags (mq_flags) that are associated with that mqd_t.

// Misc Notes:
// Persistence
//       POSIX message queues have kernel persistence: if not removed by
//       mq_unlink(3), a message queue will exist until the system is shut
//       down.
//
//   Linking
//       Programs using the POSIX message queue API must be compiled with
//       cc -lrt to link against the real-time library, librt.

#include <sdtio.h>
#include <string.h>

int main(int argc, char *argv[]){
	if (strcmp(argv[0], "rcvr")) {
		printf("Ready to receive./n");
	} 
	else if (strcmp(argv[0], "sndr")) {
		printf("Ready to send./n");
	}
	

	return 0;
}
