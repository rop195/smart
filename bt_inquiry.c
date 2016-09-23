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

#include "bt_inquiry.h"
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

void *bt_inquiry(void *arg)
{
	System_Tip         *sys_tip;
	inquiry_info       *bd_list   = NULL;
	Bd_Device          *bddevice  = NULL;
	Bd_Node            *node      = NULL;
	Bd_Node            *temp      = NULL;
	Bd_Node            *unkown    = NULL;
	Msg_Header         *head;
	Msg_Attachment     *attachment;
	Smrt_BtStatus      *btstatus;
	Message             msg;
	bdaddr_t            bdaddr;
	int                 result    = -1;
	int                 localsock = -1;
	int                 remotsock = -1;
	int                 bluetId   = -1;
	int                 bluetNum  = -1;
	int                 filesize  =  0;
	int                 filefd    = -1;
	char                btname[32];
	char                addr[19];
	uint8_t            *src;
	uint32_t            n, i;
	
	/*
	bddevice = (Bd_Device *)malloc(sizeof(Bd_Device));

	memset(bddevice->EquipUID, 0, sizeof(Bd_Device));

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

	write(filefd, bddevice->EquipUID, sizeof(Bd_Device));

	memset(bddevice->EquipUID, 0, sizeof(Bd_Device));

	bddevice->EquipMAC[0] = 0x8C;
	bddevice->EquipMAC[1] = 0xDE;
	bddevice->EquipMAC[2] = 0x52;
	bddevice->EquipMAC[3] = 0xFD;
	bddevice->EquipMAC[4] = 0x74;
	bddevice->EquipMAC[5] = 0x5A;

	memcpy(bddevice->EquipUID, "ABTJ00017000110000000001", 24);

	write(filefd, bddevice->EquipUID, sizeof(Bd_Device));

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
		pthread_mutex_lock(&sys_tip->sys_sts->list_mutex);

		filesize = get_file_size(BT_DEVICE_LIST);

		if(filesize > 0)
		{
			bddevice = (Bd_Device *)malloc(filesize);

			filefd = open(BT_DEVICE_LIST, O_RDONLY, S_IRUSR);

			read(filefd, bddevice->EquipUID, filesize);

			close(filefd);
		}

		pthread_mutex_unlock(&sys_tip->sys_sts->list_mutex);

		#ifdef DEBUG
		printf("\n");
		for(n = 0; n < filesize / sizeof(Bd_Device); n++)
		{
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
			printf("\n");
		}
		printf("\n");
		#endif

		if(sys_tip->sys_sts->g_link_status == LINK_LOGIN)
		{
			bd_list = (inquiry_info *)malloc(MAX_BLUETOOTH_NUMS * sizeof(inquiry_info));

			bluetNum = hci_inquiry(bluetId, 5, MAX_BLUETOOTH_NUMS, NULL, &bd_list, IREQ_CACHE_FLUSH);

			unkown = NULL; 

			printf("bluetNum = %d\n", bluetNum);

			if((bluetNum > 0) && (filesize > 0))
			{
				for(n = 0; n < bluetNum; n++)
				{
					src = (uint8_t *)&((bd_list + n)->bdaddr);

					ba2str(&(bd_list + n)->bdaddr, addr);

					memset(btname, 0, sizeof(btname));
        
					hci_read_remote_name(localsock, &(bd_list + n)->bdaddr, sizeof(btname), btname, 0);

					printf("Device: %s %s\n", addr, btname);

					for(i = 0; i < filesize / sizeof(Bd_Device); i++)
					{
						if(bt_MAC_cmp((bddevice + i)->EquipMAC, src) == BT_MAC_MATCH)
						{
							pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

							result = check_has_connected(sys_tip->sys_sts->bd_list_head, (bddevice + i)->EquipMAC);

							pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);

							if(result == HAS_NOT_CONNECTED)
							{
								node = (Bd_Node *)malloc(sizeof(Bd_Node));

								memset(node, 0, sizeof(Bd_Node));

								node->socketfd = -1;

								memcpy(node->EquipMAC, (bddevice + i)->EquipMAC, sizeof(node->EquipMAC));
								memcpy(node->EquipUID, (bddevice + i)->EquipUID, sizeof(node->EquipUID));
								strcpy(node->EquipName, btname);

								printf("add device: ");
								printf("%02X:",  node->EquipMAC[0]);
								printf("%02X:",  node->EquipMAC[1]);
								printf("%02X:",  node->EquipMAC[2]);
								printf("%02X:",  node->EquipMAC[3]);
								printf("%02X:",  node->EquipMAC[4]);
								printf("%02X:",  node->EquipMAC[5]);
								printf(" %s\n", node->EquipName);

								pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

								add_bd2list(&sys_tip->sys_sts->bd_list_head, &node);

								pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);
							}
							else
							{
								printf("%s has add to list\n", btname);
							}

							break;
						}
					}

					if(i >= filesize / sizeof(Bd_Device))
					{
						node = (Bd_Node *)malloc(sizeof(Bd_Node));

						memset(node, 0, sizeof(Bd_Node));

						node->EquipMAC[0] = src[5];
						node->EquipMAC[1] = src[4];
						node->EquipMAC[2] = src[3];
						node->EquipMAC[3] = src[2];
						node->EquipMAC[4] = src[1];
						node->EquipMAC[5] = src[0];

						strcpy(node->EquipName, btname);

						printf("unkown device: %s ", node->EquipName);
						printf("MAC: ");
						printf("%02X:", node->EquipMAC[0]);
						printf("%02X:", node->EquipMAC[1]);
						printf("%02X:", node->EquipMAC[2]);
						printf("%02X:", node->EquipMAC[3]);
						printf("%02X:", node->EquipMAC[4]);
						printf("%02X ", node->EquipMAC[5]);
						printf("\n");

						add_bd2list(&unkown, &node);
					}
				}
			}

			free(bd_list);

			node = sys_tip->sys_sts->bd_unkn_list;

			sys_tip->sys_sts->bd_unkn_list = unkown;

			while(node != NULL)
			{
				temp = (Bd_Node *)node->next;
			
				free(node);

				node = temp;
			}

			//
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
							close(remotsock);

							printf("can not connect %s\n", node->EquipName);
						}
					}
					else
					{
						printf("%s has connected\n", node->EquipName);
					}
				
					node = (Bd_Node *)node->next;
				}
			}
			//
		}
		else
		{
			printf("has not login\n");
		}

		if(filesize > 0)
		{
			free(bddevice);
		}
	}

	pthread_exit(NULL);
}