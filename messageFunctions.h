#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSGSIZE 512
  struct msgbuf {
      long mtype;
      char mtext[MSGSIZE];
      int priority;
      char filename[MSGSIZE];
      int bytesSent;
      pid_t processID;
  }message;

int read_message (int qid, long type, struct msgbuf *qbuf );
int send_message( int msg_qid, struct msgbuf *qbuf );
void mqstat_print (key_t mkey, int mqid, struct msqid_ds *mstat);
int initsem (key_t key);
void P(int sid);
void V(int sid);

