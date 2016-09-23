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

int bt_UID_cmp(uint8_t *src, uint8_t *dst)
{
	uint8_t n;

	for(n = 0; n < UID_LEN; n++)
	{
		if(*src++ != *dst++)
		{
			return BT_UID_NOT_MATCH;
		}
	}

	return BT_UID_MATCH;
}


