#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "protocol.h"

void *smrt_process(void *arg)
{
	int             result = 0;
	int             len    = 0;
	Message         msg;
	System_Tip      *sys_tip;

	sys_tip = (System_Tip *)arg;

	while(1)
	{
		len = msgrcv(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, MSG_TYPE_INNER, 0);

		if(len > 0)
		{
			result = protocol(msg.MsgBuf, sys_tip);

			switch(result)
			{
				case ACK:
					package(msg.MsgBuf, sys_tip);

					msg.MsgType = MSG_TYPE_PROTOCOL;

					msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
					break;

				default:
					break;
			}
		}
	}

	pthread_exit(NULL);
}