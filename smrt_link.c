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

#include "smrt_link.h"
#include "protocol.h"
#include "crc32.h"

void *smrt_link(void *arg)
{
	System_Tip      *sys_tip;
	Msg_Header      *head;
	Smrt_Login      *login;
	Msg_Attachment  *attachment;
	Message         msg;

	sys_tip = (System_Tip *)arg;

	while(1)
	{
		switch(sys_tip->sys_sts->g_link_status)
		{
			case LINK_CONNECT:
				head       = (Msg_Header *)msg.MsgBuf;
				login      = (Smrt_Login *)(msg.MsgBuf + sizeof(Msg_Header));
				attachment = (Msg_Attachment *)(msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_Login));

				memset(msg.MsgBuf, 0, sizeof(Msg_Header) + sizeof(Smrt_Login) + sizeof(sys_tip->sys_sts->SessionId));

				memcpy(head->TermCode,   sys_tip->sys_pmt->TermCode,  sizeof(sys_tip->sys_pmt->TermCode));
				memcpy(login->CloudCode, sys_tip->sys_pmt->CloudCode, sizeof(sys_tip->sys_pmt->CloudCode));
				memcpy(login->UserName,  sys_tip->sys_pmt->UserName,  sizeof(sys_tip->sys_pmt->UserName));
				memcpy(login->Password,  sys_tip->sys_pmt->Password,  sizeof(sys_tip->sys_pmt->Password));
				memcpy(login->AppUserID, sys_tip->sys_pmt->AppUserID, sizeof(sys_tip->sys_pmt->AppUserID));

				head->FrameHead       = MSG_FRAME_HEAD;
				head->TotalLen        = sizeof(Msg_Header) + sizeof(Smrt_Login) + sizeof(sys_tip->sys_sts->SessionId);
				head->CommType        = 0;
				head->MajorVer        = 0;
				head->MinorVer        = 0;	
				head->ServCode        = SERV_CODE_LOGON;
				head->ServType        = SERV_TYPE_LINK;
				head->Flags           = SESSIONID | SINGLE_FRAME;
				attachment->SessionId = sys_tip->sys_sts->SessionId;
				head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

				msg.MsgType = MSG_TYPE_PROTOCOL;
				msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);

				sleep(2);
				break;

			case LINK_LOGIN:
				pthread_mutex_lock(&sys_tip->sys_sts->argc_mutex);

				if(++sys_tip->sys_sts->g_heartbeat_tick > TICK_TIME)
				{
					sys_tip->sys_sts->g_heartbeat_tick = 0;
					
					if(++sys_tip->sys_sts->g_heartbeat_cnts > HEARTBEAT_CNTS)
					{
						sys_tip->sys_sts->g_heartbeat_cnts = 0;

						close(sys_tip->sys_sts->g_smrt_sockfd);
					}
					else
					{
						memset(msg.MsgBuf, 0, sizeof(Msg_Header));

						head = (Msg_Header *)msg.MsgBuf;
						memcpy(head->TermCode, sys_tip->sys_pmt->TermCode, sizeof(sys_tip->sys_pmt->TermCode));
						head->FrameHead = MSG_FRAME_HEAD;
						head->TotalLen  = sizeof(Msg_Header);	
						head->ServCode  = SERV_CODE_HEATBIT;
						head->ServType  = SERV_TYPE_LINK;
						head->CRC32     = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

						msg.MsgType = MSG_TYPE_PROTOCOL;
						msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
					}
				}

				pthread_mutex_unlock(&sys_tip->sys_sts->argc_mutex);

				sleep(1);
				break;

			default:
				break;
		}
	}

	pthread_exit(NULL);
}

