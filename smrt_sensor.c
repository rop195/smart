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

#include "smrt_sensor.h"
#include "protocol.h"
#include "crc32.h"

static uint16_t get_sensor_data(uint8_t type)
{
	uint16_t value = 0;

	switch(type)
	{
		case 0:
			value = 32;
			break;

		case 1:
			value = 90;
			break;

		case 2:
			value = 50;
			break;
	}

	return value;
}

void *smrt_sensor(void *arg)
{
	System_Tip      *sys_tip;
	Msg_Header      *head;
	Smrt_Sens       *sensor;
	Msg_Attachment  *attachment;
	Message          msg;
	uint32_t         tpcnts  = 0;
	uint32_t         hmcnts  = 0;
	uint32_t         aircnts = 0;

	sys_tip = (System_Tip *)arg;

	memset(msg.MsgBuf, 0, sizeof(Msg_Header) + sizeof(Smrt_Sens) + sizeof(sys_tip->sys_sts->SessionId));

	head       = (Msg_Header *)msg.MsgBuf;
	sensor     = (Smrt_Sens *)(msg.MsgBuf + sizeof(Msg_Header));
	attachment = (Msg_Attachment *)(msg.MsgBuf + sizeof(Msg_Header) + sizeof(Smrt_Sens));

	head->FrameHead = MSG_FRAME_HEAD;
	head->TotalLen  = sizeof(Msg_Header) + sizeof(Smrt_Login) + sizeof(sys_tip->sys_sts->SessionId);
	head->Flags     = SESSIONID | SINGLE_FRAME;

	while(1)
	{
		sleep(1);
		
		if(sys_tip->sys_sts->g_link_status == LINK_LOGIN)
		{
			if(++tpcnts > sys_tip->sys_pmt->TPInterval)
			{
				tpcnts = 0;

				memcpy(head->TermCode, sys_tip->sys_pmt->TermCode, sizeof(sys_tip->sys_pmt->TermCode));
				strcpy(sensor->EquipId, SMRT_EQUIPID);

				head->ServCode        = SERV_CODE_SENS;
				head->ServType        = SERV_TYPE_DQRY;
				sensor->Data          = get_sensor_data(TP_TYPE);
				sensor->Type          = TP_TYPE;
				attachment->SessionId = sys_tip->sys_sts->SessionId;
				head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

				msg.MsgType = MSG_TYPE_PROTOCOL;
				//msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
			}

			if(++hmcnts > sys_tip->sys_pmt->HMInterval)
			{
				hmcnts = 0;

				memcpy(head->TermCode, sys_tip->sys_pmt->TermCode, sizeof(sys_tip->sys_pmt->TermCode));
				strcpy(sensor->EquipId, SMRT_EQUIPID);

				head->ServCode        = SERV_CODE_SENS;
				head->ServType        = SERV_TYPE_DQRY;
				sensor->Data          = get_sensor_data(HM_TYPE);;
				sensor->Type          = HM_TYPE;
				attachment->SessionId = sys_tip->sys_sts->SessionId;
				head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

				msg.MsgType = MSG_TYPE_PROTOCOL;
				//msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
			}

			if(++aircnts > sys_tip->sys_pmt->AirInterval)
			{
				aircnts = 0;

				memcpy(head->TermCode, sys_tip->sys_pmt->TermCode, sizeof(sys_tip->sys_pmt->TermCode));
				strcpy(sensor->EquipId, SMRT_EQUIPID);

				head->ServCode        = SERV_CODE_SENS;
				head->ServType        = SERV_TYPE_DQRY;
				sensor->Data          = get_sensor_data(AIR_TYPE);;
				sensor->Type          = AIR_TYPE;
				attachment->SessionId = sys_tip->sys_sts->SessionId;
				head->CRC32           = crc32((uint8_t *)&head->SeqId, head->TotalLen - MSG_BYTES_OFFSET);

				msg.MsgType = MSG_TYPE_PROTOCOL;
				//msgsnd(sys_tip->sys_sts->g_smrt_msg, &msg, MSG_LEN, 0);
			}
		}
	}

	pthread_exit(NULL);
}