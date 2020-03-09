#include "messageFunctions.h"

int read_message (int qid, long type, struct msgbuf *qbuf )
{         
    int result, length;         // The length is essentially the size of the structure minus sizeof(mtype)         
    length = sizeof(struct msgbuf);                
    if ( (result = msgrcv ( qid, qbuf, length, type,  0) ) == -1)       
    {                
        return(-1);         
    }         
    return(result); 
} 

int send_message( int msg_qid, struct msgbuf *qbuf ) {       
    int result, length;         /* The length is essentially the size of the structure minus sizeof(mtype) */        
    length = sizeof (struct msgbuf);             
    if( (result = msgsnd ( msg_qid, qbuf, length, 0) ) == -1)       
    {                 
        return (-1);         
    }         
    return (result); 
} 

/*--------- status info print function ---------*/
void mqstat_print (key_t mkey, int mqid, struct msqid_ds *mstat)
{
  /*-- call the library function ctime ----*/
  char *ctime();

  printf ("\nKey %d, msg_qid %d\n\n", mkey, mqid);
  printf ("%d messages on queue\n\n", (int)mstat->msg_qnum);
  printf ("Last send by proc %d at %s\n",
           mstat->msg_lspid, ctime(&(mstat->msg_stime)));
  printf ("Last recv by proc %d at %s\n",
           mstat->msg_lrpid, ctime(&(mstat->msg_rtime)));
}

    
