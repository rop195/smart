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

void *smrt_tx(void *arg)
{
	int             result = 0;
	int             len    = 0;
	fd_set          wset;
	struct timeval  tv     = {1, 0};
	Message         msg;
	System_Tip      *sys_tip;
	Msg_Header      *head;

	sys_tip = (System_Tip *)arg;

	while(1)
	{
		if((sys_tip->sys_sts->g_link_status == LINK_CONNECT) || (sys_tip->sys_sts->g_link_status == LINK_LOGIN))
		{
			len = msgrcv(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, MSG_TYPE_PROTOCOL, 0);

			if(len > 0)
			{
				FD_ZERO(&wset);

				FD_SET(sys_tip->sys_sts->g_smrt_sockfd, &wset);

				tv.tv_sec  = 1;
				tv.tv_usec = 0;

				result = select((sys_tip->sys_sts->g_smrt_sockfd + 1), NULL, &wset, NULL, &tv);

				if(result == 1)
				{
					head = (Msg_Header *)msg.MsgBuf;

					write(sys_tip->sys_sts->g_smrt_sockfd, msg.MsgBuf, head->TotalLen);
				}
			}
		}
	}

	pthread_exit(NULL);
}

