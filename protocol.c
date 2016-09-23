#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "protocol.h"
#include "crc32.h"
#include "misc.h"

void package(uint8_t *buf, System_Tip *sys_tip)
{
	Msg_Attachment *attachment;
	Msg_Header     *head;

	head = (Msg_Header *)buf;

	buf += head->TotalLen;

	memset(buf, 0, sizeof(Msg_Attachment));

	attachment = (Msg_Attachment *)buf;

	memcpy(head->TermCode, sys_tip->sys_pmt->TermCode, sizeof(sys_tip->sys_pmt->TermCode));

	head->MajorVer = MAJOR_VERSION;
	head->MinorVer = MINOR_VERSION;

	if((head->Flags & SESSIONID) == SESSIONID)
	{
		attachment->SessionId = sys_tip->sys_sts->SessionId;
		head->TotalLen       += sizeof(sys_tip->sys_sts->SessionId);
	}

	head->CRC32 = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);
}

static int smrt_set_basic(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t n, *dst, *src;

	Msg_Common *msg_rsp;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	src = buf;
	dst = sys_tip->sys_pmt->TermCode;

	for(n = 0; n < sizeof(Smrt_Basic); n++)
	{
		*dst++ = *src++;
	}

	msg_rsp = (Msg_Common *)buf;

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_set_network(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t n, *dst, *src;

	Msg_Common *msg_rsp;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}
	
	src = buf;
	dst = sys_tip->sys_pmt->LanMACAddr;

	for(n = 0; n < sizeof(Smrt_Network); n++)
	{
		*dst++ = *src++;
	}

	msg_rsp = (Msg_Common *)buf;

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_set_interface(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t    *src, *dst, n;
	Msg_Common *msg_rsp;
	Smrt_Intf  *interface;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	msg_rsp   = (Msg_Common *)buf;
	interface = (Smrt_Intf *)buf;
	
	src = interface->MasterIp;
	dst = sys_tip->sys_pmt->MasterIp;

	for(n = 0; n < sizeof(Smrt_Intf); n++)
	{
		*dst++ = *src++;
	}

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_set_NTP(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t    *src, *dst, n;
	Msg_Common *msg_rsp;
	Smrt_NTP   *NTP;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	msg_rsp   = (Msg_Common *)buf;
	NTP       = (Smrt_NTP *)buf;
	
	src = NTP->NTPAddr;
	dst = sys_tip->sys_pmt->NTPAddr;

	for(n = 0; n < sizeof(Smrt_NTP); n++)
	{
		*dst++ = *src++;
	}

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_set_PKey(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t    *src, *dst, n;
	Msg_Common *msg_rsp;
	Smrt_PKey  *PKey;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	msg_rsp   = (Msg_Common *)buf;
	PKey      = (Smrt_PKey *)buf;
	
	src = &PKey->AlgorithmId;
	dst = &sys_tip->sys_pmt->AlgorithmId;

	for(n = 0; n < sizeof(Smrt_PKey); n++)
	{
		*dst++ = *src++;
	}

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_set_interval(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t        *src, *dst, n;
	Msg_Common     *msg_rsp;
	Smrt_Interval  *interval;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	msg_rsp  = (Msg_Common *)buf;
	interval = (Smrt_Interval *)buf;
	
	src = (uint8_t *)&interval->TPInterval;
	dst = (uint8_t *)&sys_tip->sys_pmt->TPInterval;

	for(n = 0; n < sizeof(Smrt_Interval); n++)
	{
		*dst++ = *src++;
	}

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_query_basic(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	Smrt_Basic  *basic;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	basic = (Smrt_Basic *)buf;

	memcpy(basic->TermCode, sys_tip->sys_pmt->TermCode, sizeof(sys_tip->sys_pmt->TermCode));
	memcpy(basic->TermDesc, sys_tip->sys_pmt->TermDesc, sizeof(sys_tip->sys_pmt->TermDesc));
	memcpy(basic->AddrDesc, sys_tip->sys_pmt->AddrDesc, sizeof(sys_tip->sys_pmt->AddrDesc));

	head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_Basic);
	head->Flags    = SESSIONID | SINGLE_FRAME;

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_query_net(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t n, *src, *dst;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	src = sys_tip->sys_pmt->LanMACAddr;
	dst = buf;

	for(n = 0; n < sizeof(Smrt_Network); n++)
	{
		*dst++ = *src++;
	}

	head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_Basic);
	head->Flags    = SESSIONID | SINGLE_FRAME;

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_query_intf(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t n, *src, *dst;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	src = sys_tip->sys_pmt->MasterIp;
	dst = buf;

	for(n = 0; n < sizeof(Smrt_Intf); n++)
	{
		*dst++ = *src++;
	}

	head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_Intf);
	head->Flags    = SESSIONID | SINGLE_FRAME;

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_query_NTP(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t n, *src, *dst;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	src = sys_tip->sys_pmt->NTPAddr;
	dst = buf;

	for(n = 0; n < sizeof(Smrt_NTP); n++)
	{
		*dst++ = *src++;
	}

	head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_NTP);
	head->Flags    = SESSIONID | SINGLE_FRAME;

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_query_PKey(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t n, *src, *dst;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	src = &sys_tip->sys_pmt->AlgorithmId;
	dst = buf;

	for(n = 0; n < sizeof(Smrt_PKey); n++)
	{
		*dst++ = *src++;
	}

	head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_PKey);
	head->Flags    = SESSIONID | SINGLE_FRAME;

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_query_videosource(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	uint8_t n, *src, *dst;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	src = sys_tip->sys_sts->RTSPServIP;
	dst = buf;

	for(n = 0; n < sizeof(Smrt_RTSPServ); n++)
	{
		*dst++ = *src++;
	}

	head->TotalLen = sizeof(Msg_Header) + sizeof(Smrt_RTSPServ);
	head->Flags    = SESSIONID | SINGLE_FRAME;

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_query_unkownbt(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	Smrt_UnkownNode   *node;
	Bd_Node           *bt;
	uint16_t           len, space;
	uint8_t            flag = 0;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	printf("%s:%d query unkown bt device\n", __FUNCTION__, __LINE__);

	space = MSG_LEN - sizeof(Msg_Header) - sizeof(sys_tip->sys_sts->SessionId);
	len   = space;

	node           = (Smrt_UnkownNode *)buf;
	bt             = sys_tip->sys_sts->bd_unkn_list;
	head->TotalLen = sizeof(Msg_Header);

	pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

	while(bt != NULL)
	{
		if(len > sizeof(Smrt_UnkownNode))
		{
			memcpy(node->EquipName, bt->EquipName, sizeof(node->EquipName));
			memcpy(node->EquipMAC,  bt->EquipMAC,  sizeof(node->EquipMAC));

			len            -= sizeof(Smrt_UnkownNode);
			head->TotalLen += sizeof(Smrt_UnkownNode);
			bt              = (Bd_Node *)bt->next;
			node++;
		}
		else
		{
			if(flag == 0)
			{
				flag++;

				head->Flags = SESSIONID | FIRST_FRAME;
			}
			else
			{
				head->Flags = SESSIONID | MIDDLE_FRAME;
			}

			package(msg->MsgBuf, sys_tip);

			msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

			len            = space;
			head->TotalLen = sizeof(Msg_Header);
			node           = (Smrt_UnkownNode *)buf;
		}
	}

	pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);

	if(flag == 0)
	{
		head->Flags = SESSIONID | SINGLE_FRAME;
	}
	else
	{
		head->Flags = SESSIONID | FINISH_FRAME;
	}

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_query_btonline(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	Smrt_BtCnntNode  *node;
	Bd_Node          *bt; 
	uint16_t          len, space;
	uint8_t           flag = 0;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	printf("%s:%d query connectd bt device\n", __FUNCTION__, __LINE__);

	space = MSG_LEN - sizeof(Msg_Header) - sizeof(sys_tip->sys_sts->SessionId);
	len   = space;

	node           = (Smrt_BtCnntNode *)buf;
	bt             = sys_tip->sys_sts->bd_list_head;
	head->TotalLen = sizeof(Msg_Header);

	pthread_mutex_lock(&sys_tip->sys_sts->blue_mutex);

	while(bt != NULL)
	{
		if(len > sizeof(Smrt_BtCnntNode))
		{
			memcpy(node->EquipUID,  bt->EquipUID,  sizeof(node->EquipUID));
			memcpy(node->EquipName, bt->EquipName, sizeof(node->EquipName));

			len            -= sizeof(Smrt_BtCnntNode);
			head->TotalLen += sizeof(Smrt_BtCnntNode);
			bt              = (Bd_Node *)bt->next;
			node++;
		}
		else
		{
			if(flag == 0)
			{
				flag++;

				head->Flags = SESSIONID | FIRST_FRAME;
			}
			else
			{
				head->Flags = SESSIONID | MIDDLE_FRAME;
			}

			package(msg->MsgBuf, sys_tip);

			msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

			len            = space;
			head->TotalLen = sizeof(Msg_Header);
			node           = (Smrt_BtCnntNode *)buf;
		}
	}

	pthread_mutex_unlock(&sys_tip->sys_sts->blue_mutex);

	if(flag == 0)
	{
		head->Flags = SESSIONID | SINGLE_FRAME;
	}
	else
	{
		head->Flags = SESSIONID | FINISH_FRAME;
	}

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_ctrl_btcon(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	Msg_Common    *msg_rsp;
	Bd_Device     *node, bddevice;
	struct stat    statbuf;
	int            filesize = 0;
	int            filefd;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	printf("%s:%d ctrl bt device connect\n", __FUNCTION__, __LINE__);

	msg_rsp  = (Msg_Common *)buf;
	node     = (Bd_Device *)buf;

	memcpy(bddevice.EquipMAC, node->EquipMAC, sizeof(bddevice.EquipMAC));
	memcpy(bddevice.EquipUID, node->EquipUID, sizeof(bddevice.EquipUID));
	memcpy(bddevice.PinCode,  node->PinCode,  sizeof(bddevice.PinCode));
	bddevice.PINLen = node->PINLen;

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	pthread_mutex_lock(&sys_tip->sys_sts->list_mutex);

	stat(BT_DEVICE_LIST, &statbuf);  
    
	filesize = statbuf.st_size;

	filefd = open(BT_DEVICE_LIST, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

	lseek(filefd, filesize, SEEK_SET);

	write(filefd, bddevice.EquipUID, sizeof(Bd_Device));

	close(filefd);

	pthread_mutex_unlock(&sys_tip->sys_sts->list_mutex);

	return ACK;
}

static int smrt_ctrl_btdel(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	Msg_Common    *msg_rsp;
	Bd_Device     *node, *temp;
	struct stat    statbuf;
	int            filesize = 0;
	int            filefd;
	uint8_t        UID[UID_LEN];
	uint32_t       n, i;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	printf("%s:%d ctrl bt device deletel\n", __FUNCTION__, __LINE__);

	msg_rsp  = (Msg_Common *)buf;
	
	memcpy(UID, buf, UID_LEN);

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	pthread_mutex_lock(&sys_tip->sys_sts->list_mutex);

	stat(BT_DEVICE_LIST, &statbuf);  
    
	filesize = statbuf.st_size;

	if(filesize > 0)
	{
		node = (Bd_Device *)malloc(filesize);

		filefd = open(BT_DEVICE_LIST, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

		read(filefd, node->EquipUID, filesize);

		temp = node;

		for(n = 0; n < filesize / sizeof(Bd_Device); n++)
		{
			if(bt_UID_cmp(UID, temp->EquipUID) == BT_UID_MATCH)
			{
				i = n + 1;

				while(i < filesize / sizeof(Bd_Device))
				{
					memcpy(temp->EquipUID, (temp + 1)->EquipUID, sizeof(temp->EquipUID));
					memcpy(temp->EquipMAC, (temp + 1)->EquipMAC, sizeof(temp->EquipMAC));
					memcpy(temp->PinCode,  (temp + 1)->PinCode,  sizeof(temp->PinCode));
					temp->PINLen = (temp + 1)->PINLen;

					i++;
					temp++;
				}

				filesize -= sizeof(Bd_Device);

				break;
			}
			else
			{
				temp++;
			}
		}

		write(filefd, node->EquipUID, filesize);

		ftruncate(filefd, filesize);

		close(filefd);
	}

	pthread_mutex_unlock(&sys_tip->sys_sts->list_mutex);
}

static int smrt_ctrl_btcmd(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	Msg_Common     *msg_rsp;
	Smrt_BtCmd     *btcmd[0];
	Message         btmsg;
	uint8_t         *src, *dst, len = 0;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	printf("%s:%d get bluetooth cmd\n", __FUNCTION__, __LINE__);

	msg_rsp  = (Msg_Common *)buf;
	btcmd[0] = (Smrt_BtCmd *)buf;
	btcmd[1] = (Smrt_BtCmd *)btmsg.MsgBuf;

	dst = btmsg.MsgBuf + sizeof(Smrt_BtCmd);
	src = buf + sizeof(Smrt_BtCmd);

	btmsg.MsgType = MSG_TYPE_BLUETOOTH;
	
	btcmd[1]->len = btcmd[0]->len;
	memcpy(btcmd[1]->EquipUID, btcmd[0]->EquipUID, sizeof(btcmd[0]->EquipUID));
	memcpy(dst, src, btcmd[0]->len);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, &btmsg, MSG_LEN, 0);

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	package(msg->MsgBuf, sys_tip);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

	return ACK;
}

static int smrt_resp_login(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	Msg_Attachment  *attachment;
	Msg_Common      *resp;

	resp = (Msg_Common *)buf;

	if(((head->Flags & SESSIONID) == SESSIONID) && (resp->ErrorCode == OK))
	{
		buf += sizeof(Msg_Common);

		attachment = (Msg_Attachment *)buf;
		
		sys_tip->sys_sts->SessionId     = attachment->SessionId;
		sys_tip->sys_sts->g_link_status = LINK_LOGIN;

		package(msg->MsgBuf, sys_tip);

		msgsnd(sys_tip->sys_sts->g_smrt_msg, msg, MSG_LEN, 0);

		printf("%s:%d get SessionId = %d\n", __FUNCTION__, __LINE__, sys_tip->sys_sts->SessionId);
	}

	return NONE_ACK;
}

static int smrt_resp_heartbeat(System_Tip *sys_tip)
{
	pthread_mutex_lock(&sys_tip->sys_sts->argc_mutex);

	sys_tip->sys_sts->g_heartbeat_cnts = 0;

	pthread_mutex_unlock(&sys_tip->sys_sts->argc_mutex);
}

static int smrt_resp(Message *msg, Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	int             result;
	Msg_Common      *msg_rsp;
	Msg_Attachment  *attachment;

	msg_rsp = (Msg_Common *)buf;

	switch(msg_rsp->RespServType)
	{
		case SERV_TYPE_LINK:
			switch(msg_rsp->RespServCode)
			{
				case SERV_CODE_LOGON:
					result = smrt_resp_login(msg, head, buf, sys_tip);
					break;

				case SERV_CODE_HEATBIT:
					result = smrt_resp_heartbeat(sys_tip);
					break;

				default:
					result = SRV_CODE_ERR;
					break;
			}
			break;

		case SERV_TYPE_PQRY:
			break;

		case SERV_TYPE_CTRL:
			break;

		case SERV_TYPE_DQRY:
			break;

		default:
			result = SRV_TYPE_ERR;
			break;
	}

	return result;
}

int protocol(Message *msg, System_Tip *sys_tip)
{
	Msg_Header  *head;
	int          result;

	head         = (Msg_Header *)msg->MsgBuf;
	msg->MsgType = MSG_TYPE_PROTOCOL;

	if(head->CRC32 == crc32(msg->MsgBuf + MSG_BYTES_OFFSET, head->TotalLen - MSG_BYTES_OFFSET))
	{
		//printf("servtype : %d\n", head->ServType);
		//printf("servcode : %d\n", head->ServCode);

		//for(result = 0; result < head->TotalLen; result++)
		//{
		//	printf("%02X ", msg->MsgBuf[result]);
		//}

		//printf("\n");

		switch(head->ServType)
		{
			case SERV_TYPE_PSET:
				switch(head->ServCode)
				{
					case SERV_CODE_BASE:
						result = smrt_set_basic(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_NET:
						result = smrt_set_network(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_INTF:
						result = smrt_set_interface(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_NTP:
						result = smrt_set_NTP(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_PKEY:
						result = smrt_set_PKey(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_SETINTERVAL:
						result = smrt_set_interval(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					default:
						result = SRV_CODE_ERR;
						break;
				}
				break;

			case SERV_TYPE_PQRY:
				switch(head->ServCode)
				{
					case SERV_CODE_BASE:
						result = smrt_query_basic(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_NET:
						result = smrt_query_net(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_INTF:
						result = smrt_query_intf(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_NTP:
						result = smrt_query_NTP(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_PKEY:
						result = smrt_query_PKey(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_VIDEOSOURCE:
						result = smrt_query_videosource(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_QRYBT:
						result = smrt_query_unkownbt(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_BTCNNT:
						result = smrt_query_btonline(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					default:
						result = SRV_CODE_ERR;
						break;
				}
				break;

			case SERV_TYPE_CTRL:
				switch(head->ServCode)
				{
					case SERV_CODE_RESET:
						break;

					case SERV_CODE_DATACLR:
						break;

					case SERV_CODE_RESTORE:
						break;

					case SERV_CODE_AJUSTTIME:
						break;

					case SERV_CODE_BTCON:
						result = smrt_ctrl_btcon(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_BTDEL:
						result = smrt_ctrl_btdel(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_BTCMD:
						result = smrt_ctrl_btcmd(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_FILERENAME:
						break;

					case SERV_CODE_FILEOP:
						break;

					case SERV_CODE_QRYFILELIST:
						break;

					default:
						result = SRV_CODE_ERR;
						break;
				}
				break;

			case SERV_TYPE_DQRY:
				switch(head->ServCode)
				{
					case SERV_CODE_VER:
						break;

					case SERV_CODE_TIME:
						break;

					default:
						result = SRV_CODE_ERR;
						break;
				}
				break;

			case SERV_TYPE_RESP:
				switch(head->ServCode)
				{
					case SERV_CODE_RESP:
						result = smrt_resp(msg, head, msg->MsgBuf + sizeof(Msg_Header), sys_tip);
						break;

					default:
						result = SRV_CODE_ERR;
						break;
				}
				break;

			default:
				result = SRV_TYPE_ERR;
				break;
		}
	}
	else
	{
		result = MSG_CRC_ERR;
	}

	return result;
}