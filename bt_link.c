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


void *bt_link(void *arg)
{
	System_Tip         *sys_tip;
	Bd_Node            *node      = NULL;
	Msg_Header         *head;
	Msg_Attachment     *attachment;
	Smrt_BtStatus      *btstatus;
	Message             msg;
	bdaddr_t            bdaddr;
	int                 result    = -1;
	int                 remotsock = -1;

	sys_tip = (System_Tip *)arg;

	msg.MsgType = MSG_TYPE_PROTOCOL;

	head       = (Msg_Header *)msg.MsgBuf;
	btstatus   = (Smrt_BtStatus *)(msg.MsgBuf + sizeof(Msg_Header));
	attachment = (Msg_Attachment *)(msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_BtStatus));

	memset(msg.MsgBuf, 0, sizeof(Msg_Header) + sizeof(Smrt_BtStatus) + sizeof(sys_tip->sys_sts->SessionId));
	btstatus->EquipStatus = BLUETOOTH_ONLINE;
	head->FrameHead       = MSG_FRAME_HEAD;
	head->TotalLen        = sizeof(Msg_Header) + sizeof(Smrt_BtStatus) + sizeof(sys_tip->sys_sts->SessionId);
	head->ServCode        = SERV_CODE_BT;
	head->ServType        = SERV_TYPE_DQRY;
	head->Flags           = SESSIONID | SINGLE_FRAME;

	while(1)
	{
		if(sys_tip->sys_sts->bd_list_head != NULL)
		{
			node = sys_tip->sys_sts->bd_list_head;

			while(node != NULL)
			{
				printf("device: %s socket = %d\n\n", node->EquipName, node->socketfd);
				
				if(node->socketfd == -1)
				{
					remotsock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

					bdaddr.b[0] = node->EquipMAC[5];
					bdaddr.b[1] = node->EquipMAC[4];
					bdaddr.b[2] = node->EquipMAC[3];
					bdaddr.b[3] = node->EquipMAC[2];
					bdaddr.b[4] = node->EquipMAC[1];
					bdaddr.b[5] = node->EquipMAC[0];

					node->bluetaddr.rc_family  = AF_BLUETOOTH;
					node->bluetaddr.rc_channel = (uint8_t)1;
					node->bluetaddr.rc_bdaddr  = bdaddr;

					result = connect(remotsock, (struct sockaddr *)&node->bluetaddr, sizeof(node->bluetaddr));

					if(result == 0)
					{
						node->socketfd = remotsock;

						memcpy(head->TermCode,     sys_tip->sys_pmt->TermCode,  sizeof(sys_tip->sys_pmt->TermCode));
						memcpy(btstatus->EquipMAC, node->EquipMAC,              sizeof(node->EquipMAC));
						memcpy(btstatus->EquipUID, node->EquipUID,              sizeof(node->EquipUID));

						attachment->SessionId = sys_tip->sys_sts->SessionId;
						head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

						msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);

						printf("connect %s\n", node->EquipName);
					}
					else
					{
						printf("can not connect %s\n", node->EquipName);
					}
				}
				else
				{
					printf("%s has connected\n", node->EquipName);
					//sleep(1);
				}
				
				node = (Bd_Node *)node->next;
			}
		}
	}
}