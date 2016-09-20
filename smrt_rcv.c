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

void *smrt_rcv(void *arg)
{
	int                 result = 0;
	int                 len    = 0;
	fd_set              rset;
	struct timeval      tv     = {1, 0};
	struct sockaddr_in  server;
	Message             msg;
	System_Tip          *sys_tip;

	sys_tip = (System_Tip *)arg;

	while(1)
	{
		memset(&server, 0, sizeof(struct sockaddr_in));

		sys_tip->sys_sts->g_smrt_sockfd = socket(AF_INET, SOCK_STREAM, 0);

		server.sin_family      = AF_INET;
		server.sin_port        = htons(sys_tip->sys_pmt->MasterPort);
		server.sin_addr.s_addr = inet_addr("120.25.152.19");
//		server.sin_addr.s_addr = inet_addr("192.168.1.6");

		result = connect(sys_tip->sys_sts->g_smrt_sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));

		if(result == 0)
		{
			printf("%s:%d connect server!\n", __FUNCTION__, __LINE__);

			sys_tip->sys_sts->g_link_status = LINK_CONNECT;

			while(1)
			{
				FD_ZERO(&rset);
		
				FD_SET(sys_tip->sys_sts->g_smrt_sockfd, &rset);

				tv.tv_sec  = 1;
				tv.tv_usec = 0;

				result = select((sys_tip->sys_sts->g_smrt_sockfd + 1), &rset, NULL, NULL, &tv);

				if(result == 0)
				{
					continue;
				}
				else
				{
					len = read(sys_tip->sys_sts->g_smrt_sockfd, msg.MsgBuf, sizeof(msg.MsgBuf));

					if(len > 0)
					{
						msg.MsgType = MSG_TYPE_INNER;

						msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
					}
					else
					{
						printf("%s:%d link disconnect!\n", __FUNCTION__, __LINE__);

						close(sys_tip->sys_sts->g_smrt_sockfd);
						
						sys_tip->sys_sts->g_smrt_sockfd = -1;
						sys_tip->sys_sts->g_link_status = LINK_DISCONNECT;

						break;
					}
				}
			}
		}
		else
		{
			printf("%s:%d can't connect server!\n", __FUNCTION__, __LINE__);

			sleep(2);
		}
	}

	pthread_exit(NULL);
}

