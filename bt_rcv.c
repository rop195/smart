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

void *bt_rcv(void *arg)
{
	System_Tip         *sys_tip;
	Bd_Node            *node, *temp_node;
	Msg_Header         *head;
	Smrt_BtUpload      *btupload;
	Smrt_BtStatus      *btstatus;
	Msg_Attachment     *attachment;
	Message             msg;
	fd_set              rset;
	struct timeval      tv     = {1, 0};
	int                 result;
	int                 len;
	int                 socketfd;
	uint32_t            n;
	uint8_t             buf[128], *src;

	sys_tip = (System_Tip *)arg;

	msg.MsgType = MSG_TYPE_PROTOCOL;

	head     = (Msg_Header *)msg.MsgBuf;
	btupload = (Smrt_BtUpload *)(msg.MsgBuf + sizeof(Msg_Header));
	btstatus = (Smrt_BtStatus *)(msg.MsgBuf + sizeof(Msg_Header));
	src      = msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_BtUpload);

	memset(msg.MsgBuf, 0, sizeof(Msg_Header));
	head->FrameHead = MSG_FRAME_HEAD;
	head->Flags     = SESSIONID | SINGLE_FRAME;

	while(1)
	{
		while(sys_tip->sys_sts->bd_list_head != NULL)
		{
			node     = sys_tip->sys_sts->bd_list_head;
			socketfd = node->socketfd;

			FD_ZERO(&rset);

			while(node != NULL)
			{
				FD_SET(node->socketfd, &rset);

				if(node->socketfd > socketfd)
				{
					socketfd = node->socketfd;
				}

				node = (Bd_Node *)node->next;
			}

			tv.tv_sec  = 1;
			tv.tv_usec = 0;

			result = select((socketfd + 1), &rset, NULL, NULL, &tv);

			if(result == 0)
			{
				continue;
			}
			else
			{
				node = sys_tip->sys_sts->bd_list_head;

				while(node != NULL)
				{
					if(FD_ISSET(node->socketfd, &rset))
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
							
							printf("rcv: ");
	
							for(n = 0; n < len; n++)
							{
								printf("%02X ", buf[n]);
							}

							printf("\n");
						}
						else
						{
							attachment = (Msg_Attachment *)(msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_BtStatus));

							head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_BtStatus) + sizeof(sys_tip->sys_sts->SessionId);
							head->ServCode = SERV_CODE_BT;
							head->ServType = SERV_TYPE_DQRY;

							memcpy(head->TermCode,     sys_tip->sys_pmt->TermCode,  sizeof(sys_tip->sys_pmt->TermCode));
							memcpy(btstatus->EquipMAC, node->EquipMAC,              sizeof(node->EquipMAC));
							memcpy(btstatus->EquipUID, node->EquipUID,              sizeof(node->EquipUID));

							printf("lost MAC ");
							printf("%02X:", node->EquipMAC[0]);
							printf("%02X:", node->EquipMAC[1]);
							printf("%02X:", node->EquipMAC[2]);
							printf("%02X:", node->EquipMAC[3]);
							printf("%02X:", node->EquipMAC[4]);
							printf("%02X\n", node->EquipMAC[5]);

							btstatus->EquipStatus = BLUETOOTH_OFFLINE;
							attachment->SessionId = sys_tip->sys_sts->SessionId;
							head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

							msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);

							temp_node = NULL;

							pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

							close(node->socketfd);

							if((Bd_Node *)node->pre != NULL)
							{
								temp_node       = (Bd_Node *)node->pre;
								temp_node->next = (Bd_Node *)node->next;
							}
							else
							{
								sys_tip->sys_sts->bd_list_head = NULL;
							}

							free(node);

							node = temp_node;

							pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);

							printf("len = %d, offline\n", len);
						}
					}
					
					if(node != NULL)
					{
						node = (Bd_Node *)node->next;
					}
				}

				if(sys_tip->sys_sts->bd_list_head == NULL)
				{
					break;
				}
			}
		}
	}
}