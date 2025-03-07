#ifndef _CMMCOMMONSTRUCT_H
#define _CMMCOMMONSTRUCT_H
#include "Data.h"
#include "list"
#include "Poco/Timestamp.h"
#define CMM_ID_LENGTH			12
#define CMM_NAME_LENGTH			80
#define CMM_DES_LENGTH			40
#define CMM_NMALARMID_LEN		40
#define CMM_DEVICEID_LEN		26
#define CMM_VER_LENGTH			20
#define CMM_TIME_LEN			19

	//01~09  11~18 20,68,76,77,78,87,88,92,93,95
	/*
#define Is_range(iID) (iID>=1&&iID<=9)||(iID>=11&&iID<=20)||iID==68||(iID>=76&&iID<=78) ||(iID>=87&&iID<=88) ||(iID>=92&&iID<=93)||(iID==95)
#define Is_rangeData(iID) (iID>=1&&iID<=9)||(iID>=11&&iID<=18)||iID==68||(iID>=76&&iID<=78) ||(iID>=87&&iID<=88) ||(iID>=92&&iID<=93)||(iID==95)
#define Is_rangeThreshold(iID) (iID>=1&&iID<=9)||(iID>=11&&iID<=20)||iID==68||(iID>=76&&iID<=78) ||(iID>=87&&iID<=88) ||(iID>=92&&iID<=93)*/

#define Is_range(iID) ((iID>=1&&iID<=9)||(iID>=11&&iID<=18)||iID==68||(iID>=76&&iID<=78) ||(iID>=87&&iID<=88) ||(iID>=92 && iID<=96))
#define Is_rangeData(iID) Is_range(iID)
#define Is_rangeThreshold(iID) (iID==20) //Is_range(iID)
#define Is_rangeAlarm(iID) ((iID>=1&&iID<=10)||(iID>=11&&iID<=20)||iID==68||(iID>=76&&iID<=78) ||(iID>=87&&iID<=88) ||(iID>=92&&iID<=93)||(iID==95))

namespace CMM{
	namespace param{
	
	/*	const char* const xx = "经度";
		const char* const yy = "纬度";*/
		
		const char* const AlarmSendDB = "alarm-sent-status-db"; //告警发送状态数据库
		const char* const CsvEncoding = "csv-character-encoding";  //csv编码
		const char* const CsvExpire = "csv-expire-period";  //csv过期时间
		const char* const CsvMeasurementTime = "csv-measurement-period";  // AI历史存储周期(分钟): csv上报周期

		const char* const SCDoorIp = "door-server-ip";
		const char* const SCDoorPort = "door-server-port";
		const char* const SCDoorTransPort = "door-trans-port";
		const char* const SCProtocol = "door-trans-protocol";

		const char* const AuthEnable = "enable-auth";             //身份认证
		const char* const EnginState = "enable-enginnering-state";//启用工程状态（禁止上报告警）:
		const char* const UpdateEnable = "enable-realtime-update"; //启动自动更新
		const char* const SoapEnable = "enable-soap";               //soap功能

		const char* const FlowControl = "flow-control";               //流控
		const char* const RebootEnable = "fsu-auto-reboot-enable";  //自动重启开关	
		const char* const RebootPeriod = "fsu-auto-reboot-period";  //自动重启周期（天）
		const char* const RebootTime = "fsu-auto-reboot-time";      //重启具体时间
		const char* const FsuDeviceId = "fsu-device-id";
		const char* const FsuId  = "fsu-id";             //FSU ID:
		const char* const FsuEndPoint = "fsu-webservice-url";           //fsu 服务端URL http://192.168.1.168:8080/v1/services/newFSUService"

		const char* const FtpUsr = "ftp-user";
		const char* const FtpPasswd = "ftp-pass";
		const char* const FtpType = "ftp-type";

		const char* const LoggerChannel = "logger-channel";
		const char* const LoggerCount = "logger-file-count";
		const char* const LogFileSize = "logger-file-size";   // 日志大小(Mb);
		const char* const LogLevel = "logger-level";        //"日志级别";

		const char* const LoginPeriod = "re-login-period";           // 登陆重试周期（s）;
		const char* const LoginHeart = "re-login-heart";           // 心跳（s）;
		
		const char* const Algorithm = "sc-login-algorithm";  //密码算法
		const char* const Password = "sc-login-pass";     //SC Web服务登录密码
		const char* const UserName = "sc-login-user";    //SC Web服务登录用户
		const char* const SCEndPoint = "sc-webservice-url";           //sc 服务端URL http://192.168.1.184:9080/v1/services/newLSCService"

		const char* const SendPeriod = "send-conf-period";          //配置上报周期(s)";
		const char* const UpdateInterval = "update-fsuinfo-interval";   // FSU信息更新周期(秒):
		
		const char* const WebDeviceConfig = "web-api-device-config";  //Web API设备厂家信息数据库:web-api-device-props-db
		const char* const WebQueueDepth = "web-api-event-queue-depth";    //	Web API请求队列深度
		const char* const WebInterval = "web-api-event-time-interval";    //	Web API事件检查周期(秒):
		const char* const WebHost = "web-api-host";    //Web API主机/IP:
		const char* const WebHtdocs = "web-api-htdocs";    //Web API静态内容:
		const char* const WebMimes = "web-api-mimes";    //	Web API MIME
		const char* const WebPort = "web-api-port";    //	Web API端口
		const char* const WebStorageDevice = "web-api-storage-device";    //Web API存储设备:
		const char* const WebVersion = "web-api-version";    //WEBAPI 版本

		const char* const SiteID = "site-id";            //站点编号
		const char* const SiteName = "site-name";		 //站点名称
		const char* const RoomID = "room-id";			 //机房编号
		const char* const RoomName = "room-name";		 //机房名称
		const char* const UartName = "serial-port";
		const char* const BaudRate = "baud-rate";
		const char* const DataBit = "data-bits";
		const char* const Parity = "parity"; 
		const char* const StopBit = "stop-bits";
		const char* const SlaveID = "slave-id";


		const char* const LoginState= "login-state"; //注册状态
		const char* const DoorLoginState= "door-login-state"; //门禁系统注册状态
		
	}
const char* const Request = "Request";
const char* const Response = "Response";
const char* const PK_Type = "PK_Type";
const char* const Name = "Name";
const char* const Info = "Info";
const char* const UserName = "UserName";
const char* const PassWord = "PassWord";
const char* const AlgType = "AlgType";
const char* const FSUID = "FSUID";
const char* const FSUIP = "FSUIP";
const char* const FSUMAC = "FSUMAC";
const char* const FSUVER = "FSUVER";
const char* const RightLevel = "RightLevel";
const char* const Values = "Values";
const char* const DeviceList = "DeviceList";
const char* const Device = "Device";
const char* const ID = "ID";
const char* const TDevConfdesc = "TDevConfdesc";
const char* const Time = "Time";

namespace method{
	const char* const LOGIN = "LOGIN";
	const char* const LOGIN_ACK = "LOGIN_ACK";
	const char* const SEND_DEV_CONF_DATA = "SEND_DEV_CONF_DATA";
	const char* const SEND_DEV_CONF_DATA_ACK = "SEND_DEV_CONF_DATA_ACK";
	const char* const GET_DEV_CONF = "GET_DEV_CONF";
	const char* const GET_DEV_CONF_ACK = "GET_DEV_CONF_ACK";
	const char* const SET_DEV_CONF_DATA = "SET_DEV_CONF_DATA";
	const char* const SET_DEV_CONF_DATA_ACK = "SET_DEV_CONF_DATA_ACK";
	const char* const GET_FSUINFO = "GET_FSUINFO";
	const char* const GET_FSUINFO_ACK = "GET_FSUINFO_ACK";
	const char* const GET_DATA = "GET_DATA";
	const char* const GET_DATA_ACK = "GET_DATA_ACK";
	const char* const TIME_CHECK = "TIME_CHECK";
	const char* const TIME_CHECK_ACK = "TIME_CHECK_ACK";
	const char* const SEND_ALARM = "SEND_ALARM";
	const char* const SEND_ALARM_ACK = "SEND_ALARM_ACK";
	const char* const SET_POINT = "SET_POINT";
	const char* const SET_POINT_ACK = "SET_POINT_ACK";
	const char* const GET_THRESHOLD = "GET_THRESHOLD";
	const char* const GET_THRESHOLD_ACK = "GET_THRESHOLD_ACK";
	const char* const SET_THRESHOLD = "SET_THRESHOLD";
	const char* const SET_THRESHOLD_ACK = "SET_THRESHOLD_ACK";
	const char* const SEND_DATA = "SEND_DATA";
	const char* const SEND_DATA_ACK = "SEND_DATA_ACK";
	const char* const GET_FTP ="GET_FTP";
	const char* const GET_FTP_ACK = "GET_FTP_ACK";
	const char* const SET_FTP = "SET_FTP";
	const char* const SET_FTP_ACK = "SET_FTP_ACK";
	const char* const SET_LOGININFO = "SET_LOGININFO";
	const char* const SET_LOGININFO_ACK = "SET_LOGININFO_ACK";
	const char* const GET_LOGININFO = "GET_LOGININFO";
	const char* const GET_LOGININFO_ACK = "GET_LOGININFO_ACK";
	const char* const UPDATE_FSUINFO_INTERVAL = "UPDATE_FSUINFO_INTERVAL";
	const char* const UPDATE_FSUINFO_INTERVAL_ACK = "UPDATE_FSUINFO_INTERVAL_ACK";
	//canyon
	const char* const GET_STORAGERULE = "GET_STORAGERULE";
	const char* const GET_STORAGERULE_ACK = "GET_STORAGERULE_ACK";
	const char* const SET_STORAGERULE = "SET_STORAGERULE";
	const char* const SET_STORAGERULE_ACK = "SET_STORAGERULE_ACK";

	//NEW 
	const char* const GET_TIME = "GET_TIME";
	const char* const GET_TIME_ACK = "GET_TIME_ACK";
	const char* const SET_ACCEPT_IP_CONF = "SET_ACCEPT_IP_CONF";
	const char* const SET_ACCEPT_IP_CONF_ACK = "SET_ACCEPT_IP_CONF_ACK";
	const char* const SET_FSUREBOOT = "SET_FSUREBOOT";
	const char* const SET_FSUREBOOT_ACK = "SET_FSUREBOOT_ACK";
}

enum EnumResult
{
	FAILURE = 0,
	SUCCESS = 1,
	ILLEGALACCESS = 2,
	AUTHERROR = 3,
	NODATA = 4,
	UNCONFIG = 5,
	UNLOGIN = 6,
};

enum EnumRightMode
{
	INVALID = 0,
	LEVEL1 = 1,
	LEVEL2 = 2,
};
enum EnumType
{
	DI = 4,  //数字输入 遥信
	AI = 3,  //模拟输入 遥测
	DO = 1,  //数字输出 遥控
	AO = 2,  //模拟输出 遥调
	ALARM = 0,  //告警
};

enum EnumState
{
	STATE_NOALARM = 0,
	STATE_INVALID = 1,
};

enum EnumFlag
{
	BEGIN,
	END,
};

typedef struct  	sTSignal
{
	public:
		sTSignal()
		{
			Type = -1; AlarmLevel=-1; savePeriod = -1; SignalNumber = 0;
			Threshold = 0.001; AbsoluteVal = 0.001; RelativeVal = 0.001; SetupVal = 0.001; result = 0;
		}
	int Type;
	CData ID;
	CData SignalName;
	int AlarmLevel;
	float Threshold;
	float AbsoluteVal;
	float RelativeVal;
	int savePeriod; //canyon
	CData Describe;
	CData NMAlarmID;
	int SignalNumber;
	float SetupVal;
	int result;
}TSignal;

typedef struct		  sTDevConf
{
	CData DeviceID;
	CData DeviceName;
	CData RoomName;
	CData SiteName;
	CData RoomID;
	CData SiteID;
	CData DeviceType;
	CData DeviceSubType;
	CData Model;
	CData Brand;
	float RatedCapacity;
	CData Version;
	CData BeginRunTime;
	CData DevDescribe;
	CData ConfRemark;
	std::list<TSignal> singals;
	int result;
}TDevConf;

typedef struct  
{
	int Type;
	CData ID;
	float MeasuredVal;
	float SetupVal;
	int Status;
	CData Time;
	int SignalNumber;
	int result;
	int AlarmLevel; //写性能文件判定条件 xml不需要该参数
}TSemaphore;

typedef struct
{
	int Years;
	int Month;
	int Day;
	int Hour;
	int Minute;
	int Second;
}TTime;

typedef struct  
{
	CData SerialNo;
	CData ID;
	CData DeviceID;
	CData NMAlarmID;
	CData AlarmTime;
	int AlarmLevel;
	CData AlarmFlag;
	CData AlarmDesc;
	float EventValue;
	CData AlarmRemark1;
	CData AlarmRemark2;
	int SignalNumber;
	int retryTimes;
	Poco::Timestamp lastReportTime;
}TAlarm;

typedef struct
{
	int Type;
	CData ID;
	CData NMAlarmID;
	float Threshold;
	float AbsoluteVal;
	float RelativeVal;
	int AlarmLevel;
	int Status;
	int SignalNumber;
	int result;
}TThreshold;

typedef struct
{
	CData Status;
	CData lastHeartBeatTime;
	int RightLevel;
}TServerStatus;

typedef struct
{
	CData DeviceNo;   //设备编码
	CData AliasDeviceNo;   //设备别名编码
	CData DeviceName; //设备名称
	CData AliasDeviceName;//设备别名
	CData ParentDeviceID;//父设备ID
	CData DeviceSubType;//类型子设备
	CData Brand;//设备品牌
	CData Model;//设备型号
	CData Desc;//描述
	CData RatedCapacity;//额定容量
	CData Version;//版本
	CData BeginRunTime;//启用时间
}TDeviceInfo;
}
#endif
