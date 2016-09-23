#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

#include <stdint.h>
#include <pthread.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define  SMRT_EQUIPID            "SMRTGEAR"
#define  SMRT_CLOUDCODE          "3214"
#define  SMRT_USERNAME           "myname"
#define  SMRT_PASSWORD           "mypassword"
#define  SMRT_APPUSERID          "rop195@163.com  "         
#define  SMRT_TERMCODE           "321409000001"

#define  BT_DEVICE_LIST          "./bt_deivice_list"

#define  DEBUG

#define  MSG_BYTES_OFFSET        8

#define  MSG_LEN                 1024
#define  UID_LEN                 24

#define  MSG_FRAME_HEAD          0xAAFF

#define  TYPE_TCP                0
#define  TYPE_UDP                1

#define  BLUETOOTH_ONLINE        0
#define  BLUETOOTH_OFFLINE       1

#define  BT_MAC_NOT_MATCH        0
#define  BT_MAC_MATCH            1

#define  BT_UID_NOT_MATCH        0
#define  BT_UID_MATCH            1

#define  SESSIONID               0x20
#define  SINGLE_FRAME            0x06
#define  FIRST_FRAME             0x04
#define  MIDDLE_FRAME            0x00
#define  FINISH_FRAME            0X02
#define  HAS_PASSWORD            0x10
#define  HAS_TIMESTAMP           0x08

#define  MAJOR_VERSION           0
#define  MINOR_VERSION           0

#define  OK                      0
#define  ACK                     0
#define  NONE_ACK                1
#define  MSG_CRC_ERR            -1
#define  SRV_TYPE_ERR           -2
#define  SRV_CODE_ERR           -3

#define  LINK_DISCONNECT         0
#define  LINK_CONNECT            1
#define  LINK_LOGIN              2

#define  MSG_TYPE_PROTOCOL       1
#define  MSG_TYPE_INNER          2
#define  MSG_TYPE_BLUETOOTH      3

#define SERV_TYPE_RESP           1              //通用报文类型
#define SERV_CODE_RESP           1              //通用报文代码

#define SERV_TYPE_LINK           2              //链路检测
#define SERV_CODE_LOGON          1              //登录
#define SERV_CODE_LOGOFF         2              //退出
#define SERV_CODE_HEATBIT        3              //心跳

#define SERV_TYPE_PSET           3              //参数设置
#define SERV_TYPE_PQRY           4              //参数查询
#define SERV_CODE_BASE           1              //基本信息
#define SERV_CODE_NET            2              //网络参数
#define SERV_CODE_INTF           3              //接口参数
#define SERV_CODE_NTP            4              //时钟参数
#define SERV_CODE_FTP            5              //!!!!!!!
#define SERV_CODE_PKEY           6              //终端密码
#define SERV_CODE_SETINTERVAL    7              //传感器采集周期设置
#define SERV_CODE_VIDEOSOURCE    7              //视频源地址查询
#define SERV_CODE_QRYBT          9              //可见未知蓝牙设备列表查询
#define SERV_CODE_BTCNNT         11             //已连接的蓝牙设备列表

#define SERV_TYPE_CTRL           5              //命令控制
#define SERV_CODE_RESET          1              //系统复位
#define SERV_CODE_DATACLR        2              //数据清理
#define SERV_CODE_RESTORE        3              //恢复出厂
#define SERV_CODE_AJUSTTIME      4              //时间校准
#define SERV_CODE_BTCON          5              //连接指定蓝牙设备
#define SERV_CODE_BTDEL          6              //删除指定蓝牙设备
#define SERV_CODE_BTCMD          7              //向指定蓝牙设备发送指令
#define SERV_CODE_FILERENAME     8              //重命名文件
#define SERV_CODE_FILEOP         9              //操作文件
#define SERV_CODE_QRYFILELIST    10             //查询文件列表

#define SERV_TYPE_DQRY           6              //数据查询
#define SERV_CODE_SENS           1              //传感器状态
#define SERV_CODE_VER            2              //版本及状态信息
#define SERV_CODE_TIME           3              //当前日期及时间
#define SERV_CODE_EXTSENS        5              //蓝牙传感设备数据
#define SERV_CODE_REGBT          6        	    //注册蓝牙设备
#define SERV_CODE_BT             7        	    //蓝牙设备上线下线通知

#define SERV_TYPE_FILE           7      	    //文件传输
#define SERV_CODE_FILE_UPREQ     1              //文件上传请求
#define SERV_CODE_FILE_DOWNREQ   2              //文件下载请求
#define SERV_CODE_FILE_DATA      3              //文件传输请求

#pragma  pack(1)

typedef struct 
{
	uint8_t EquipName[32];
	uint8_t EquipMAC[6];
	uint8_t EquipUID[24];
	struct  sockaddr_rc  bluetaddr;
	int     socketfd;
	void    *pre;
	void    *next;
}Bd_Node;

typedef struct 
{
	uint8_t EquipUID[24];
	uint8_t EquipMAC[6];
	uint8_t PINLen;
	uint8_t PinCode[16];
}Bd_Device;

typedef struct 
{
	uint32_t TPInterval;
	uint32_t HMInterval;
	uint32_t AirInterval;

	uint8_t TermCode[12];
	uint8_t TermDesc[32];
	uint8_t AddrDesc[64];

	uint8_t AlgorithmId;                 
	uint8_t Key[16];

	uint8_t LanMACAddr[6];                  
	uint8_t LanIPAddr[4];                   
	uint8_t LanIPMask[4];                  
	uint8_t LanGWAddr[4];                  
	uint8_t LanAutoIP;                      
	uint8_t WiFiMACAddr[6];                
	uint8_t WiFiIPAddr[4];                 
	uint8_t WiFiIPMask[4];                 
	uint8_t WiFiGWAddr[4];                 
	uint8_t WiFiAutoIP;                     
	uint8_t DNS1[4];                         
	uint8_t DNS2[4];

	uint8_t  MasterIp[4];                  
	uint16_t MasterPort;                  
	uint8_t  SlaveIp[4];                   
	uint16_t SlavePort;                   
	uint8_t  UserName[32];                 
	uint8_t  Password[16];                  
	uint8_t  CommType;                     
	uint8_t  HbPeriod;                     
	uint8_t  ResendNum;                
	uint8_t  Timeout;

	uint8_t  NTPAddr[32];
	uint16_t NTPPort;					
	uint8_t  SyncPeriod;					
	uint8_t  EnableFlag;

	uint8_t CloudCode[4];          
	uint8_t AppUserID[16];
}System_Paramt;

typedef struct 
{
	pthread_mutex_t argc_mutex;
	pthread_mutex_t blue_mutex;
	pthread_mutex_t list_mutex;
	uint32_t        SessionId;
	uint32_t        g_heartbeat_tick;
	uint32_t        g_heartbeat_cnts;
	int             g_smrt_sockfd;
	int             g_link_status;
	int             g_smrt_msg;

	uint8_t         password[16];
	uint8_t         RTSPServIP[4];
	uint16_t        RTSPServPort;
	Bd_Node         *bd_list_head;
	Bd_Node         *bd_unkn_list;
}System_Status;

typedef struct 
{
	System_Paramt  *sys_pmt;
	System_Status  *sys_sts; 
}System_Tip;

typedef struct 
{
	long    MsgType;
	uint8_t MsgBuf[MSG_LEN];
}Message;

typedef struct 
{
	uint16_t FrameHead;
	uint16_t TotalLen;
	uint32_t CRC32;           
	uint32_t SeqId;       
	uint8_t  CommType;           
	uint8_t  EncType;        
	uint8_t  MajorVer;        
	uint8_t  MinorVer;      
	uint8_t  TermCode[12];
	uint16_t ServType;
	uint16_t ServCode;
	uint8_t  Flags;
	uint8_t  Priority;
	uint8_t  Reserved[4]; 
}Msg_Header;

typedef struct 
{
	uint32_t  SessionId;
	uint8_t   Password[16];
	uint8_t   Timestamp[6];
	int       Expiretime;
}Msg_Attachment;

typedef struct 
{
	uint16_t RespServType;
	uint16_t RespServCode;
	int      ErrorCode;
}Msg_Common;

typedef struct 
{
	uint8_t CloudCode[4];          
	uint8_t UserName[32];                 
	uint8_t Password[16];
	uint8_t AppUserID[16];
}Smrt_Login;

typedef struct 
{
	uint8_t TermCode[12];
	uint8_t TermDesc[32];
	uint8_t AddrDesc[64];
}Smrt_Basic;

typedef struct 
{
	uint8_t LanMACAddr[6];                  
	uint8_t LanIPAddr[4];                   
	uint8_t LanIPMask[4];                  
	uint8_t LanGWAddr[4];                  
	uint8_t LanAutoIP;                      
	uint8_t WiFiMACAddr[6];                
	uint8_t WiFiIPAddr[4];                 
	uint8_t WiFiIPMask[4];                 
	uint8_t WiFiGWAddr[4];                 
	uint8_t WiFiAutoIP;                     
	uint8_t DNS1[4];                         
	uint8_t DNS2[4];  
}Smrt_Network;

typedef struct 
{
	uint8_t  RTSPServIP[4];
	uint16_t RTSPServPort;
}Smrt_RTSPServ;

typedef struct 
{
	uint32_t TPInterval;
	uint32_t HMInterval;
	uint32_t AirInterval;
}Smrt_Interval;

typedef struct 
{
	uint8_t  EquipId[16];
	uint16_t Data;
	uint8_t  Type;
}Smrt_Sens;

typedef struct 
{
	uint8_t  MasterIp[4];                  
	uint16_t MasterPort;                  
	uint8_t  SlaveIp[4];                   
	uint16_t SlavePort;                   
	uint8_t  UserName[32];                 
	uint8_t  Password[16];                  
	uint8_t  CommType;                     
	uint8_t  HbPeriod;                     
	uint8_t  ResendNum;                
	uint8_t  Timeout;
}Smrt_Intf;

typedef struct 
{
	uint8_t  NTPAddr[32];					//密码算法编号
	uint16_t NTPPort;						//NTP服务器侦听端口（缺省：123）
	uint8_t  SyncPeriod;					//同步周期，单位：天
	uint8_t  EnableFlag;
}Smrt_NTP;

typedef struct 
{
	uint8_t AlgorithmId;                   //密码算法编号
	uint8_t Key[16];
}Smrt_PKey;

typedef struct 
{
	uint8_t EquipUID[24];
	uint8_t EquipMAC[6];
	uint8_t EquipStatus;
}Smrt_BtStatus;

typedef struct 
{
	uint8_t  EquipUID[24];
	uint32_t len;
}Smrt_BtCmd;

typedef struct 
{
	uint8_t  EquipUID[24];
	uint8_t  EquipMAC[6];
	uint16_t FrameNum;
}Smrt_BtUpload;

typedef struct 
{
	uint8_t EquipUID[24];
	uint8_t EquipName[32];
}Smrt_BtCnntNode;

typedef struct 
{
	uint8_t EquipName[32];
	uint8_t EquipMAC[6];
}Smrt_UnkownNode;


extern int protocol(Message *msg, System_Tip *sys_tip);

#endif
