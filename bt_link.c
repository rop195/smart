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

#include "bt_link.h"
#include "protocol.h"
#include "crc32.h"

static int get_file_size(char *filename)  
{  
    struct stat statbuf;
	int         size;

    stat(filename, &statbuf);  
    
	size = statbuf.st_size;  
  
    return size;  
}

static int bt_MAC_cmp(uint8_t *src, uint8_t *dst)
{
	uint8_t n;

	for(n = 0; n < 6; n++)
	{
		if(src[n] != dst[5 - n])
		{
			return BT_MAC_NOT_MATCH;
		}
	}

	return BT_MAC_MATCH;
}

static void add_bd2list(Bd_Node **head, Bd_Node **node)
{
	Bd_Node *crt, *next;

	uint8_t n;

	if(*head == NULL)
	{
		*head = *node;
	}
	else
	{
		crt  = *head;
		next = (Bd_Node *)((*head)->next);

		while(next != NULL)
		{
			crt  = next;
			next = (Bd_Node *)next->next;
		}

		crt->next    = *node;
		(*node)->pre = crt;
	}
}

static int check_has_connected(Bd_Node *head, uint8_t *MAC)
{
	Bd_Node *temp;

	uint8_t n;

	if(head == NULL)
	{
		return HAS_NOT_CONNECTED;
	}
	else
	{
		temp = head;

		do{
			for(n = 0; n < sizeof(head->EquipMAC); n++)
			{
				if(temp->EquipMAC[n] != MAC[n])
				{
					break;
				}
			}

			if(n >= sizeof(head->EquipMAC))
			{
				return HAS_CONNECTED;
			}

			temp = (Bd_Node *)temp->next;

		}while(temp != NULL);

		return HAS_NOT_CONNECTED;
	}
}

void *bt_link(void *arg)
{
	System_Tip         *sys_tip;
	inquiry_info       *bd_list   = NULL;
	Bd_Device          *bddevice  = NULL;
	Bd_Node            *node      = NULL;
	Msg_Header         *head;
	Smrt_BtStatus      *btstatus;
	Msg_Attachment     *attachment;
	struct sockaddr_rc  bluetaddr = {0};
	Message             msg;
	int                 result    = -1;
	int                 localsock = -1;
	int                 remotsock = -1;
	int                 bluetId   = -1;
	int                 bluetNum  = -1;
	int                 filesize  =  0;
	int                 filefd    = -1;
	char                btname[32];
	char                addr[19];
	uint8_t             btdevcnt  =  0;
	uint8_t            *src;
	uint32_t            n, i, btcnctnum = 0;

	uint8_t             cmd[] = {0xcc, 0x80, 0x02, 0x03, 0x01, 0x01, 0x00, 0x01};
	//uint8_t             cmd[] = {0xcc, 0x80, 0x02, 0x03, 0x01, 0x02, 0x00, 0x02};
	//uint8_t             cmd[] = {0xcc, 0x80, 0x02, 0x03, 0x01, 0x04, 0x00, 0x04};
	
	/*
	bddevice = (Bd_Device *)malloc(sizeof(Bd_Device));

	memset(bddevice->EquipMAC, 0, sizeof(Bd_Device));

	bddevice->EquipMAC[0] = 0x00;
	bddevice->EquipMAC[1] = 0x0E;
	bddevice->EquipMAC[2] = 0x0E;
	bddevice->EquipMAC[3] = 0x11;
	bddevice->EquipMAC[4] = 0x5F;
	bddevice->EquipMAC[5] = 0xC4;

	bddevice->PINLen = 4;

	bddevice->PinCode[0] = 1; 
	bddevice->PinCode[1] = 2; 
	bddevice->PinCode[2] = 3; 
	bddevice->PinCode[3] = 4; 

	memcpy(bddevice->EquipUID, "AATJ00017000310000000002", 24);

	filefd = open(BT_DEVICE_LIST, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

	write(filefd, bddevice->EquipMAC, sizeof(Bd_Device));

	memset(bddevice->EquipMAC, 0, sizeof(Bd_Device));

	bddevice->EquipMAC[0] = 0x8C;
	bddevice->EquipMAC[1] = 0xDE;
	bddevice->EquipMAC[2] = 0x52;
	bddevice->EquipMAC[3] = 0xFD;
	bddevice->EquipMAC[4] = 0x74;
	bddevice->EquipMAC[5] = 0x5A;

	memcpy(bddevice->EquipUID, "ABTJ00017000110000000001", 24);

	write(filefd, bddevice->EquipMAC, sizeof(Bd_Device));

	close(filefd);

	free(bddevice);
	*/

	sys_tip = (System_Tip *)arg;

	bd_list = (inquiry_info *)malloc(MAX_BLUETOOTH_NUMS * sizeof(inquiry_info));
	
	sys_tip->sys_sts->bd_list_head = NULL;

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
		bluetId   = hci_get_route(NULL);
		localsock = hci_open_dev(bluetId);

		if((bluetId < 0) || (localsock < 0))
		{
			printf("open socket error!\n");

			sleep(2);
		}
		else
		{
			break;
		}
	}
	
	while(1)
	{
		filesize = get_file_size(BT_DEVICE_LIST);

		if(filesize > 0)
		{
			bddevice = (Bd_Device *)malloc(filesize);

			filefd = open(BT_DEVICE_LIST, O_RDONLY, S_IRUSR);

			read(filefd, bddevice->EquipMAC, filesize);

			close(filefd);
		}

		#ifdef DEBUG
		for(n = 0; n < filesize / sizeof(Bd_Device); n++)
		{
			printf("\n");
			printf("bluetooth device: %d\n", n);
			printf("MAC: ");
			printf("%02X:",  (bddevice + n)->EquipMAC[0]);
			printf("%02X:",  (bddevice + n)->EquipMAC[1]);
			printf("%02X:",  (bddevice + n)->EquipMAC[2]);
			printf("%02X:",  (bddevice + n)->EquipMAC[3]);
			printf("%02X:",  (bddevice + n)->EquipMAC[4]);
			printf("%02X\n", (bddevice + n)->EquipMAC[5]);

			printf("UID: ");
			for(i = 0; i < 24; i++)
			{
				printf("%C", (bddevice + n)->EquipUID[i]);
			}
			printf("\n\n");
		}
		#endif

		if(sys_tip->sys_sts->g_link_status == LINK_LOGIN)
		{
			bluetNum = hci_inquiry(bluetId, 5, MAX_BLUETOOTH_NUMS, NULL, &bd_list, IREQ_CACHE_FLUSH);

			#ifdef DEBUG
			if(bluetNum > 0)
			{
				printf("\nhas found:\n");
			
				for(n = 0; n < bluetNum; n++)
				{
					ba2str(&(bd_list + n)->bdaddr, addr);

					memset(btname, 0, sizeof(btname));
        
					hci_read_remote_name(localsock, &(bd_list + n)->bdaddr, sizeof(btname), btname, 0);
				
					printf("Device: %s %s\n", addr, btname);
				}
			}
			#endif

			if((bluetNum > 0) && (filesize > 0))
			{
				for(n = 0; n < bluetNum; n++)
				{
					src = (uint8_t *)&(bd_list + n)->bdaddr;

					for(i = 0; i < filesize / sizeof(Bd_Device); i++)
					{
						if(bt_MAC_cmp(src, (bddevice + i)->EquipMAC) == BT_MAC_MATCH)
						{
							pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

							result = check_has_connected(sys_tip->sys_sts->bd_list_head, (bddevice + i)->EquipMAC);

							pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);

							if(result == HAS_NOT_CONNECTED)
							{
								memset(btname, 0, sizeof(btname));

								hci_read_remote_name(localsock, &(bd_list + n)->bdaddr, sizeof(btname), btname, 0);

								remotsock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

								bluetaddr.rc_family  = AF_BLUETOOTH;
								bluetaddr.rc_channel = (uint8_t)1;
								bluetaddr.rc_bdaddr  = (bd_list + n)->bdaddr;

								result = connect(remotsock, (struct sockaddr *)&bluetaddr, sizeof(bluetaddr));

								if(result == 0)
								{
									printf("%d\n", write(remotsock, cmd, 8));

									printf("connect %s!\n", btname);

									node = (Bd_Node *)malloc(sizeof(Bd_Node));

									memset(node, 0, sizeof(Bd_Node));

									node->socketfd = remotsock;

									memcpy(node->EquipMAC, (bddevice + i)->EquipMAC, sizeof(node->EquipMAC));
									memcpy(node->EquipUID, (bddevice + i)->EquipUID, sizeof(node->EquipUID));
									strcpy(node->EquipName, btname);

									printf("node: ");
									printf("%02X:",  node->EquipMAC[0]);
									printf("%02X:",  node->EquipMAC[1]);
									printf("%02X:",  node->EquipMAC[2]);
									printf("%02X:",  node->EquipMAC[3]);
									printf("%02X:",  node->EquipMAC[4]);
									printf("%02X\n", node->EquipMAC[5]);

									printf("node: %s\n", node->EquipName);

									pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

									add_bd2list(&sys_tip->sys_sts->bd_list_head, &node);

									pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);

									memcpy(head->TermCode,     sys_tip->sys_pmt->TermCode,  sizeof(sys_tip->sys_pmt->TermCode));
									memcpy(btstatus->EquipMAC, node->EquipMAC,              sizeof(node->EquipMAC));
									memcpy(btstatus->EquipUID, node->EquipUID,              sizeof(node->EquipUID));

									attachment->SessionId = sys_tip->sys_sts->SessionId;
									head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

									msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
								}
								else
								{
									printf("can't connect %s\n", btname);
								}
							}
							else
							{
								printf("has connected\n");
							}
							break;
						}
					}
				}
			}
		}

		if(filesize > 0)
		{
			free(bddevice);
		}
	}

	free(bd_list);

	pthread_exit(NULL);
}