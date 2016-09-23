#include <stdio.h>
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

#include "protocol.h"
#include "smrt_rcv.h"
#include "smrt_tx.h"
#include "smrt_link.h"
#include "smrt_sensor.h"
#include "smrt_process.h"
#include "bt_inquiry.h"
#include "bt_rcv.h"
#include "bt_tx.h"

#define  NIF_MSG_QUEUE_FILE  "./tmp"

static int smrt_msg_init(int *g_smrt_msg)
{
	key_t   key; 
	int     fd;
	int     id = 0;

	if((fd = open(NIF_MSG_QUEUE_FILE, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
	{
		printf("%s %d, open message queue fail! \n", __FUNCTION__, __LINE__);
		return -1;
	}

	if((key = ftok(NIF_MSG_QUEUE_FILE, 0xFF)) == -1)
	{
		printf("%s %d, ftok fail! \n", __FUNCTION__, __LINE__);
		return -1;
	}

	if((*g_smrt_msg = msgget(key, IPC_CREAT | IPC_EXCL | 0666)) == -1)
	{
		printf("%s %d, msgget! \n", __FUNCTION__, __LINE__);

        id = msgget(key, IPC_CREAT | 0666);
        
		if(id == -1)
        {
            printf("%s %d, msgget! \n", __FUNCTION__, __LINE__);
            return -2;
        }
        else
        {
            msgctl(id, IPC_RMID, NULL);
        }

		if((*g_smrt_msg = msgget(key, IPC_CREAT | 0666)) == -1)
        {
            printf("%s %d, msgget fail! \n", __FUNCTION__, __LINE__);
            return -1;
        }
	}

	return 0;
}

static void system_init(System_Tip *sys_tip)
{
	memset((uint8_t *)&sys_tip->sys_pmt->TPInterval, 0, sizeof(System_Paramt));

	strcpy(sys_tip->sys_pmt->CloudCode, "3214"); 
	strcpy(sys_tip->sys_pmt->UserName,  "hwtest01"); 
	strcpy(sys_tip->sys_pmt->Password,  "hwpassed01");
	strcpy(sys_tip->sys_pmt->AppUserID, "hwtest01");
	strcpy(sys_tip->sys_pmt->TermCode,  "321409000003");

	sys_tip->sys_pmt->MasterPort       =  20000;
	sys_tip->sys_pmt->TPInterval       =  10;
	sys_tip->sys_pmt->HMInterval       =  25;
	sys_tip->sys_pmt->AirInterval      =  30;

	sys_tip->sys_sts->SessionId        =  1;
	sys_tip->sys_sts->g_heartbeat_cnts =  0;
	sys_tip->sys_sts->g_heartbeat_tick =  0;
	sys_tip->sys_sts->g_smrt_sockfd    = -1;
	sys_tip->sys_sts->RTSPServPort     =  504;
	sys_tip->sys_sts->RTSPServIP[0]    =  192;
	sys_tip->sys_sts->RTSPServIP[1]    =  168;
	sys_tip->sys_sts->RTSPServIP[2]    =  0;
	sys_tip->sys_sts->RTSPServIP[3]    =  105;
	sys_tip->sys_sts->bd_list_head     =  NULL;
	sys_tip->sys_sts->bd_unkn_list     =  NULL;
}

int main()
{
	pthread_t       tid[9];

	System_Paramt   sys_paramt;
	System_Status   sys_status;
	System_Tip      sys_tip;

	sys_tip.sys_pmt = &sys_paramt;
	sys_tip.sys_sts = &sys_status;

	system_init(&sys_tip);

	smrt_msg_init(&sys_tip.sys_sts->g_smrt_msg);
	
	pthread_mutex_init(&sys_tip.sys_sts->argc_mutex, NULL);
	pthread_mutex_init(&sys_tip.sys_sts->blue_mutex, NULL);
	pthread_mutex_init(&sys_tip.sys_sts->list_mutex, NULL);

	pthread_create(&tid[0], NULL, smrt_rcv,      &sys_tip);
	pthread_create(&tid[1], NULL, smrt_tx,       &sys_tip);
	pthread_create(&tid[2], NULL, smrt_link,     &sys_tip);
	pthread_create(&tid[3], NULL, smrt_sensor,   &sys_tip);
	pthread_create(&tid[4], NULL, smrt_process,  &sys_tip);
	pthread_create(&tid[5], NULL, bt_inquiry,    &sys_tip);
	pthread_create(&tid[6], NULL, bt_rcv,        &sys_tip);
	pthread_create(&tid[7], NULL, bt_tx,         &sys_tip);

	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[2], NULL);
	pthread_join(tid[3], NULL);
	pthread_join(tid[4], NULL);
	pthread_join(tid[5], NULL);
	pthread_join(tid[6], NULL);
	pthread_join(tid[7], NULL);
}
