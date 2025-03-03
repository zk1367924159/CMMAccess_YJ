#ifndef _CMMPARAM_H
#define _CMMPARAM_H
#include "Data.h"
#include "NetComm/CXmlElement.h"
#include "CMMCommonStruct.h"
#include "SmartLock.h"
#include "json.hpp"
using json = nlohmann::json;

namespace CMM{
	class CMMParam{
	
	public:
		static CMMParam* instance();
		bool Init();
		void initParam();
		bool writeJson2File();
		void initJsonFile();
		json& GetJsonObj();
		void SetJsonObj(json& jsonObj);
		int UpdateParam(CData key, CData val);
		CData GetParam(CData key, CData defVal = "");
		int SetParam(CData key, CData val);
	public:
		static CMMParam *_instance;
		json m_json;
        CData m_CsvEncoding;  //csv编码 GBK UTF
        CData m_CsvExpire;    //csv过期时间 (天)
        CData m_CsvMeasurementTime; //csv上报周期 分钟
        CData m_ScIp;   // 门禁服务IP
        CData m_ScPort; // 门禁服务端口
        CData m_ScUdpPort; // 门禁透传端口
        CData m_ScProtocol; // 门禁透传协议 TCP/UDP
        CData m_AuthEnable; //是否开启SC服务认证
        CData m_EnginState;  //启用工程状态（禁止上报告警）
        CData m_UpdateEnable; //实时数据更新通知
        CData m_SoapEnable;   //soap开关
        CData m_RebootEnable;  //自动重启开关
        
        CData m_RebootPeriod;  //自动重启周期（天)
        CData m_RebootTime;    //自动重启详细时间
        CData m_FsuId;         //
        CData m_FsuEndPoint;    //FSU Web服务URL
        CData m_FtpUsr;        //
        CData m_FtpPasswd;
        CData m_FtpType;       // FTP/SFTP
        CData m_LogFileSize;
        CData m_LogLevel;
        CData m_LoginPeriod;   //登陆周期（注册失败重试时间 s）
		CData m_LoginHeart;    //登录心跳
        CData m_Algorithm;     //密码算法: 
        CData m_Password;
        CData m_UserName;
        CData m_ScEndPoint;    //SC Web服务URL:
        CData m_SendPeriod;    //配置上报周期(分钟):
        CData m_UpdateInterval;  //FSU信息更新周期(秒):
        
        CData m_WebDevicedDB;
        CData m_WebQueueDepth;
        CData m_WebInterval;
        CData m_WebHost;   //WEBAPI 主机IP
        CData m_WebHtdocs;
        CData m_WebMimes;
        CData m_WebPort;  //WEBAPIport
        CData m_WebStorageDevice;
        CData m_WebVersion;  //Web API版本
        
        CData m_SiteID;			 //站点编号
        CData m_SiteName;		 //站点名称
        CData m_RoomID;			 //机房编号
        CData m_RoomName;		 //机房名称
        CData m_UartName;
        CData m_BaudRate;
        CData m_DataBit;
        CData m_Parity;
        CData m_StopBit;
        CData m_SlaveID;

		CData m_scUdpPoint;
	};
}
#endif
