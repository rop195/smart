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

static int bt_MAC_cmp(uint8_t *src, uint8_t *dst)
{
	uint8_t n;

	for(n = 0; n < 6; n++)
	{
		if(*src++ != *dst++)
		{
			return BT_MAC_NOT_MATCH;
		}
	}

	return BT_MAC_MATCH;
}

void *bt_tx(void *arg)
{
	System_Tip      *sys_tip;
	Smrt_BtCmd      *btcmd;
	Bd_Node         *node;
	Message          msg;
	int              len;
	
	sys_tip = (System_Tip *)arg;

	while(1)
	{
		len = msgrcv(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, MSG_TYPE_BLUETOOTH, 0);

		if(len > 0)
		{
			btcmd = (Smrt_BtCmd *)msg.MsgBuf;

			pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

			node  = sys_tip->sys_sts->bd_list_head;

			while(node != NULL)
			{
				if(bt_MAC_cmp(btcmd->EquipMAC, node->EquipMAC) == BT_MAC_MATCH)
				{
					write(node->socketfd, msg.MsgBuf + sizeof(Smrt_BtCmd), btcmd->len);

					break;
				}

				node = (Bd_Node *)node->next;
			}

			pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);
		}
	}
}

