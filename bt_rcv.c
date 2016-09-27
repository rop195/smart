#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "protocol.h"
#include "crc32.h"
#include "bt_rcv.h"
#include "misc.h"

void *bt_rcv(void *arg)
{
	Bd_Link            *link;
	System_Tip         *sys_tip;
	Bd_Node            *node, *temp;
	Msg_Header         *head;
	Smrt_BtUpload      *btupload;
	Smrt_BtStatus      *btstatus;
	Msg_Attachment     *attachment;
	uint8_t            *src, buf[128];
	bdaddr_t            bdaddr;
	Message             msg;
	fd_set              rset;
	struct timeval      tv     = {1, 0};
	struct sockaddr_rc  bluetaddr;
	int                 result;
	int                 remotsock;
	int                 len;
	uint32_t            n;

	pthread_detach(pthread_self());

	link = (Bd_Link *)arg;

	sys_tip      = link->sys_tip;

	bdaddr.b[0]  = link->MAC[5];
	bdaddr.b[1]  = link->MAC[4];
	bdaddr.b[2]  = link->MAC[3];
	bdaddr.b[3]  = link->MAC[2];
	bdaddr.b[4]  = link->MAC[1];
	bdaddr.b[5]  = link->MAC[0];

	msg.MsgType = MSG_TYPE_PROTOCOL;

	head     = (Msg_Header *)msg.MsgBuf;
	btupload = (Smrt_BtUpload *)(msg.MsgBuf + sizeof(Msg_Header));
	btstatus = (Smrt_BtStatus *)(msg.MsgBuf + sizeof(Msg_Header));
	src      = msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_BtUpload);

	memset(msg.MsgBuf, 0, sizeof(Msg_Header));
	head->FrameHead = MSG_FRAME_HEAD;
	head->Flags     = SESSIONID | SINGLE_FRAME;

	node = sys_tip->sys_sts->bd_list_head;

	result = BT_MAC_NOT_MATCH;

	pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

	while(node != NULL)
	{
		result = bt_MAC_cmp(node->EquipMAC, bdaddr.b);

		if(result == BT_MAC_MATCH)
		{
			break;
		}
		else
		{
			node = (Bd_Node *)node->next;
		}
	}

	pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);

	if(result == BT_MAC_MATCH)
	{
		remotsock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		bluetaddr.rc_family  = AF_BLUETOOTH;
		bluetaddr.rc_channel = (uint8_t)1;
		bluetaddr.rc_bdaddr  = bdaddr;

		result = connect(remotsock, (struct sockaddr *)&bluetaddr, sizeof(bluetaddr));

		if(result == 0)
		{
			printf("connect %s successful!\n", node->EquipName);

			node->socketfd = remotsock;

			attachment = (Msg_Attachment *)(msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_BtStatus));

			head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_BtStatus) + sizeof(sys_tip->sys_sts->SessionId);
			head->ServCode = SERV_CODE_BT;
			head->ServType = SERV_TYPE_DQRY;

			memcpy(head->TermCode,     sys_tip->sys_pmt->TermCode,  sizeof(sys_tip->sys_pmt->TermCode));
			memcpy(btstatus->EquipMAC, node->EquipMAC,              sizeof(node->EquipMAC));
			memcpy(btstatus->EquipUID, node->EquipUID,              sizeof(node->EquipUID));
			
			btstatus->EquipStatus = BLUETOOTH_ONLINE;
			attachment->SessionId = sys_tip->sys_sts->SessionId;
			head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

			msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);

			while(1)
			{
				FD_ZERO(&rset);

				FD_SET(node->socketfd, &rset);

				tv.tv_sec  = 1;
				tv.tv_usec = 0;

				result = select((node->socketfd + 1), &rset, NULL, NULL, &tv);

				if(result > 0)
				{
					len = read(node->socketfd, buf, sizeof(buf));

					if(len > 0)
					{
						head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_BtUpload) + len + sizeof(sys_tip->sys_sts->SessionId);
						head->ServCode = SERV_CODE_EXTSENS;
						head->ServType = SERV_TYPE_DQRY;

						memcpy(head->TermCode,     sys_tip->sys_pmt->TermCode,  sizeof(sys_tip->sys_pmt->TermCode));
						memcpy(btupload->EquipUID, node->EquipUID,              sizeof(node->EquipUID));
						memcpy(btupload->EquipMAC, node->EquipMAC,              sizeof(node->EquipMAC));
						memcpy(src,                buf,                         len);

						btupload->FrameNum = len;

						attachment = (Msg_Attachment *)(msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_BtUpload) + len);

						attachment->SessionId = sys_tip->sys_sts->SessionId;
						head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

						msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
							
						printf("get from %s: ", node->EquipName);
	
						for(n = 0; n < len; n++)
						{
							printf("%02X ", buf[n]);
						}

						printf("\n");
					}
					else
					{
						close(node->socketfd);

						printf("close socket = %d\n", node->socketfd);

						attachment = (Msg_Attachment *)(msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_BtStatus));

						head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_BtStatus) + sizeof(sys_tip->sys_sts->SessionId);
						head->ServCode = SERV_CODE_BT;
						head->ServType = SERV_TYPE_DQRY;
	
						memcpy(head->TermCode,     sys_tip->sys_pmt->TermCode,  sizeof(sys_tip->sys_pmt->TermCode));
						memcpy(btstatus->EquipMAC, node->EquipMAC,              sizeof(node->EquipMAC));
						memcpy(btstatus->EquipUID, node->EquipUID,              sizeof(node->EquipUID));
			
						btstatus->EquipStatus = BLUETOOTH_OFFLINE;
						attachment->SessionId = sys_tip->sys_sts->SessionId;
						head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

						msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
	
						printf("lost dev: %s ", node->EquipName);
						printf("MAC ");
						printf("%02X:",  node->EquipMAC[0]);
						printf("%02X:",  node->EquipMAC[1]);
						printf("%02X:",  node->EquipMAC[2]);
						printf("%02X:",  node->EquipMAC[3]);
						printf("%02X:",  node->EquipMAC[4]);
						printf("%02X\n", node->EquipMAC[5]);

						break;
					}
				}
			}
		}
		else
		{
			printf("can not connect!\n");
		}
		
		pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

		if((Bd_Node *)node->pre != NULL)
		{
			temp       = (Bd_Node *)node->pre;
			temp->next = (Bd_Node *)node->next;

			if((Bd_Node *)temp->next != NULL)
			{
				((Bd_Node *)(temp->next))->pre = temp;
			}
		}
		else
		{
			sys_tip->sys_sts->bd_list_head = (Bd_Node *)node->next;

			if((Bd_Node *)node->next != NULL)
			{
				((Bd_Node *)(node->next))->pre = NULL;
			}
		}

		free(node);

		pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);
	}
	else
	{
		printf("%02X:", bdaddr.b[0]);
		printf("%02X:", bdaddr.b[1]);
		printf("%02X:", bdaddr.b[2]);
		printf("%02X:", bdaddr.b[3]);
		printf("%02X:", bdaddr.b[4]);
		printf("%02X ", bdaddr.b[5]);

		printf("MAC do not match!\n");
	}

	free(link);

	pthread_exit(NULL);
}

			

			
