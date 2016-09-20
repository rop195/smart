#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>

#include "protocol.h"
#include "crc32.h"

static int smrt_set_basic(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_set_network(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_set_interface(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_set_NTP(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_set_PKey(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_set_interval(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_query_basic(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_query_net(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_query_intf(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_query_NTP(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_query_PKey(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_query_videosource(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

	return ACK;
}

static int smrt_ctrl_btcmd(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
{
	Msg_Common     *msg_rsp;
	Message         msg;
	uint8_t         len = 0;

	if(sys_tip->sys_sts->g_link_status != LINK_LOGIN)
	{
		return NONE_ACK;
	}

	printf("%s:%d get bluetooth cmd\n", __FUNCTION__, __LINE__);

	msg.MsgType = MSG_TYPE_BLUETOOTH;

	if((head->Flags & SESSIONID) == SESSIONID)
	{
		len += sizeof(sys_tip->sys_sts->SessionId);
	}

	if((head->Flags & HAS_PASSWORD) == HAS_PASSWORD)
	{
		len += sizeof(sys_tip->sys_sts->password);
	}

	if((head->Flags & HAS_TIMESTAMP) == HAS_TIMESTAMP)
	{
		len += 6;
	}

	memcpy(msg.MsgBuf, buf, head->TotalLen - sizeof(Msg_Header) - len);

	msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);

	msg_rsp->RespServType = head->ServType;
	msg_rsp->RespServCode = head->ServCode;
	msg_rsp->ErrorCode    = OK;

	head->ServType = SERV_TYPE_RESP;
	head->ServCode = SERV_CODE_RESP;
	head->Flags    = SESSIONID | SINGLE_FRAME;
	head->TotalLen = sizeof(Msg_Header) + sizeof(Msg_Common);

	return ACK;
}

static int smrt_resp_login(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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

static int smrt_resp(Msg_Header *head, uint8_t *buf, System_Tip *sys_tip)
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
					result = smrt_resp_login(head, buf, sys_tip);
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

int protocol(uint8_t *buf, System_Tip *sys_tip)
{
	Msg_Header  *head;

	int result;

	head = (Msg_Header *)buf;

	if(head->CRC32 == crc32(buf + MSG_BYTES_OFFSET, head->TotalLen - MSG_BYTES_OFFSET))
	{
		printf("servtype : %d\n", head->ServType);
		printf("servcode : %d\n", head->ServCode);

		for(result = 0; result < head->TotalLen; result++)
		{
			printf("%x ", buf[result]);
		}

		printf("\n");

		switch(head->ServType)
		{
			case SERV_TYPE_PSET:
				switch(head->ServCode)
				{
					case SERV_CODE_BASE:
						result = smrt_set_basic(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_NET:
						result = smrt_set_network(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_INTF:
						result = smrt_set_interface(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_NTP:
						result = smrt_set_NTP(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_PKEY:
						result = smrt_set_PKey(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_SETINTERVAL:
						result = smrt_set_interval(head, buf + sizeof(Msg_Header), sys_tip);
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
						result = smrt_query_basic(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_NET:
						result = smrt_query_net(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_INTF:
						result = smrt_query_intf(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_NTP:
						result = smrt_query_NTP(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_PKEY:
						result = smrt_query_PKey(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_VIDEOSOURCE:
						result = smrt_query_videosource(head, buf + sizeof(Msg_Header), sys_tip);
						break;

					case SERV_CODE_QRYBT:
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
						break;

					case SERV_CODE_BTDEL:
						break;

					case SERV_CODE_BTCMD:
						result = smrt_ctrl_btcmd(head, buf + sizeof(Msg_Header), sys_tip);
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
						result = smrt_resp(head, buf + sizeof(Msg_Header), sys_tip);
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