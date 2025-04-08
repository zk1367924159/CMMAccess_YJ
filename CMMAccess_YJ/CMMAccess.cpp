
//canyon 2021 07 15 translate to extapp mode

#include "CMMAccess.h"
#include "CMMConfig.h"
#include "CMMParam.h"
#include "CMMDeviceConfig.h"
#include "CMMCommonStruct.h"
#include "SysCommon.h"
#include "CLog.h"
#include "CMMProtocolEncode.h"
#include "CMMMeteTranslate.h"
#include "CMMSoapXmlEncode.h"
#include "CTextEncryption.h"
#include "Poco/File.h" 
#include "Poco/Path.h" 
#include "Poco/DirectoryIterator.h"
#include "Poco/DateTimeParser.h"
#include "Poco/DateTime.h"
#include <ctime>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <regex>
#include "Poco/RegularExpression.h"
#include "../../ExtAppIpc/ExtAppIpcApi.h"
//#include "ExtSoApi.h"
#include "../../Business/DeviceMng.h"

const unsigned int CMM_MAX_RESPONSE_BUFFER_SIZE = 512*1024;


int SetAODOCb(CData devId, CData meterId, CData val)
{
	LogNotice("devId:" << devId << " meterId:" << meterId << " val:" << val);
	return 0;
}

namespace CMM{

	CMMAccess* CMMAccess::_instance = NULL;
	Poco::FastMutex CMMAccess::m_mutex;


	CMMAccess::CMMAccess()
	{
		m_bStart = false;
		m_bLoginOK = false;
		m_registerStatus = CMM_REGISTER_FAILED;
		m_udpRegisterStatus = CMM_REGISTER_FAILED;
		m_RightLevel = -1;
		m_nRetry = 0;

		m_doorServer = new DoorServerManger();
		m_doorClient = new DoorClient();
		m_webServer = new WebServer();
		m_client = new HttpClient();
		m_server = new HttpServer();
		m_uartService = new CMMUart();
	}
	
	CMMAccess* CMMAccess::instance()
	{
		if(_instance == NULL){
			Poco::FastMutex::ScopedLock lock(m_mutex);
			if(_instance == NULL){
				_instance = new CMMAccess();
			}
		}
		return _instance;
	}

	
	CData CMMAccess::GetIfcIp(CData ifcName)
	{		
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(struct ifreq));
		strcpy(ifr.ifr_name, ifcName.c_str());
		
		struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
		addr->sin_family = AF_INET;
		
		int fd = socket(AF_INET, SOCK_DGRAM, 0);

		CData ipaddr;
		if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
		{
			ipaddr = inet_ntoa(addr->sin_addr);
		}
		close(fd);
		return ipaddr;
	}

	
	CData CMMAccess::GetLocalMac()  
	{  
		CData mac;
	    int sock_mac;  
	      
	    struct ifreq ifr_mac;  
	    char mac_addr[30];     
	      
	    sock_mac = socket( AF_INET, SOCK_STREAM, 0 );  
	    if( sock_mac == -1)  
	    {  
	        LogError("create socket falise...mac/n");  
	        return mac;
	    }  
	      
	    memset(&ifr_mac,0,sizeof(ifr_mac));     
	    strncpy(ifr_mac.ifr_name, "eth0", sizeof(ifr_mac.ifr_name)-1);     
	  
	    if( (ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)  
	    {  
	        LogError("mac ioctl error/n");  
	        return mac;
	    }  
	      
	    sprintf(mac_addr,"%02x:%02x:%02x:%02x:%02x:%02x",  
	            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],  
	            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],  
	            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],  
	            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],  
	            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],  
	            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);  
	  
 	    close(sock_mac);  
	    mac = mac_addr;  
		return mac;
	}
	
	CData CMMAccess::GetNetIfcParam(CData ifconfig, CData key)
	{
		CData val;
		CData content = ifconfig;		
		CData pattern = key;
		
		int startPos = content.find(pattern);
		if (CDATA_NPOS != startPos)
		{
			int endPos =  content.find(" ", startPos+pattern.size());
			if (CDATA_NPOS != endPos)
			{
				val = content.substring(startPos+pattern.size(), endPos);
			}
		}
		return val;
	}
	
	
	void CMMAccess::AddRoute(CData destIp, CData gateWay)
	{
		
		if (gateWay == "eth0Gw")
		{
			//NETMODULE::T_NETCONFIGINFO cfg;
			//NETMODULE::GetNetCfg(cfg);
			//gateWay = cfg.localGw;
		}
		else if (gateWay == "ppp0Gw")
		{
			gateWay = GetNetIfcParam(ISFIT::Shell("ifconfig ppp0"), "P-t-P:");
		}
		else if (gateWay == "ppp1Gw")
		{
			gateWay = GetNetIfcParam(ISFIT::Shell("ifconfig ppp1"), "P-t-P:");
		}
	
		if (gateWay.size() > 0)
		{
			CData delRoute = "route del " + destIp;
			LogInfo("===>AddRoute() delRoute: "<<delRoute);
			ISFIT::Shell(delRoute.c_str());
					
			CData addRoute = "route add -net " + destIp + " netmask 255.255.255.255 gw " + gateWay;
			LogInfo("===>addRoute: "<<addRoute);
			ISFIT::Shell(addRoute.c_str());
		}
	
	}


	void CMMAccess::SetLoginState(bool isLoginOk)
	{
		if (!isLoginOk)
		{
			m_registerStatus = CMM_REGISTER_FAILED;
		}
		else
		{
			m_registerStatus = CMM_REGISTER_SUCCESS;
		}
		CData state = isLoginOk ? "注册成功" : "未注册";
		CMMParam::instance()->SetParam(CMM::param::LoginState, state);
		CData val = isLoginOk ? "0" : "1";
		APPAPI::SetMeterVal(CData("215001"), CData("138123001"), "msj",val);
	}

	void CMMAccess::SetUdpLoginState(bool isLoginOk)
	{
		if (!isLoginOk)
		{
			m_udpRegisterStatus = CMM_REGISTER_FAILED;
		}
		else
		{
			m_udpRegisterStatus = CMM_REGISTER_SUCCESS;
		}
		CData state = isLoginOk ? "注册成功" : "未注册";
		CMMParam::instance()->SetParam(CMM::param::DoorLoginState, state);
	}

	void CMMAccess::Test()
	{
		CData devID;
		//APPAPI::DelDev("069500000000001");
		while (0)
		{
			devID = "069500000000001";
			//APPAPI::DelDev("069500000000002");
			//devID = APPAPI::CreateDev("移动B接口测试设备","移动B接口测试",true,"", SetAODOCb);

			std::map<CData, CData> paramMaps;
			paramMaps["aliasDevName"] = "999999";
			paramMaps["aliasDevId"] = "999999";
			paramMaps["gatewayId"] = "local";
			int ret = APPAPI::SetDevParam(devID, "alias", paramMaps);
			LogInfo("============SetDevParam return" << ret);
			/*std::list<CData> meterIdList;
			meterIdList.push_back("001018001");
			int ret = APPAPI::CreateMeter(devID, meterIdList);
			LogInfo("============CreateMeter return " << ret);*/
		
			std::list<std::map<CData,CData> > paramList;
				std::map <CData, CData> meterParamMap;
				meterParamMap["meterId"] = "001018001";
				meterParamMap["threshold"] = CData(88);
				meterParamMap["alarmLevel"] = CData(2);
				paramList.push_back(meterParamMap);
			std::list<CData> errorMeterIdList;
			ret = APPAPI::SetMeterParam(devID, "msj", paramList, errorMeterIdList, 5000);
			LogInfo("============SetMeterParam return:"<<ret);


			ret = APPAPI::SetMeterVal(devID, "001018001", "msj", CData(0), true);
			LogInfo("============SetMeterVal return: " << ret);


			std::map<CData,CData> paramMap;
			ret = APPAPI::GetMeterParam(devID, "001018001", "msj",paramMap);
			LogInfo("============GetMeterParam " << paramMap.size() << " ret:" << ret);

			for(auto it=paramMap.begin(); it!=paramMap.end(); it++)
			{
				LogInfo("==key:"<<it->first<<" val:"<<it->second);
			}

			std::set<CData> attrSet;
			attrSet.insert("meterId");
			attrSet.insert("meterType");
			attrSet.insert("alarmLevel");
			attrSet.insert("threshold");
			//attrSet.insert("meterName");
			std::list<std::map<CData,CData> > infoList;
			ret = APPAPI::GetMeterInfo(devID, "msj", attrSet, infoList, 10000);
			LogInfo("============GetMeterInfo" << infoList.size() << " ret:" << ret);
			for (auto mit = infoList.begin(); mit != infoList.end(); mit++)
			{
				std::map<CData, CData>& attr = *mit;
				CData meterId = attr["meterId"];
				for (auto it = attr.begin(); it != attr.end(); it++)
				{
					if (meterId == "001018001")
						LogInfo("==key:" << it->first << " val:" << it->second);
				}
			}
			Poco::Thread::sleep(3000);
		}

		if (1)
		{
			LogInfo("============test2===========");
			/*Poco::SharedPtr<ISFIT::CSmartRwLock> lock;
			std::list<BUSINESS::CDevice*> DevList;
			BUSINESS::CDeviceMng::Instance().GetAllDev(DevList,lock);
			LogInfo("DevList: " << DevList.size());
			for (auto it = DevList.begin(); it != DevList.end(); it++)
			{
				BUSINESS::CDevice* pDevice = *it;
				if (pDevice)
				{
					std::list<BUSINESS::tChildDev> childDevList;
					pDevice->GetChildDevInfo(childDevList);
					if (childDevList.size() > 1)
					{
						LogInfo(pDevice->GetDevId().c_str() <<" have childDevList: " << (int)childDevList.size());
					}
					
				}
			}*/
			
		}
		
	}

	void CMMAccess::UpdateModuleInfo()
	{
		CData fsuId = CMMParam::instance()->m_FsuId;
		APPAPI::SetMeterVal(CData("215001"), CData("138127001"), "msj", fsuId);
	}

	void threadFunction2(void* arg)
	{
		CMMAccess* pThis = static_cast<CMMAccess*>(arg);
		pThis->instance()->runEx();
	}

	void CMMAccess::runEx()
	{
		bool isFirst = true;
		int errCount = 0;
		while (m_bStart)
		{
			Poco::Timestamp now;
			time_t diff = now.epochTime() - m_lastMsgTimeBak.epochTime();
			if (diff >= CMMParam::instance()->m_LoginPeriod.convertInt() || isFirst)  //发送心跳
			{
				isFirst = false;
				if (0 == m_doorClient->SendHeart(CMMParam::instance()->m_ScUdpPoint.c_str(), CMMParam::instance()->m_ScProtocol))
				{
					m_lastMsgTimeBak.update();
					SetUdpLoginState(true);
					errCount = 0;
				}
				else
				{
					errCount++;
				}
			}
			if (errCount >= 10) //超时 下线
			{
				errCount = 0;
				m_doorClient->Stop();
				m_lastMsgTimeBak.update();
				SetUdpLoginState(false);
			}

			Poco::Thread::sleep(1000);
		}
	}

	void CMMAccess::run()
	{
		SetLoginState(false);
		UpdateModuleInfo();
		bool isFirst = true;
		while (m_bStart)
		{
			//Test();
			//Poco::Timestamp now;
			//time_t diff = now.epochTime() - m_lastMsgTime.epochTime();
			//if (CMM_REGISTER_FAILED == m_registerStatus)
			//{
			//	if (isFirst || (diff >=  CMMParam::instance()->m_LoginPeriod.convertInt() && m_nRetry <= 3))
			//	{
			//		Login();
			//		m_lastMsgTime.update();
			//		m_nRetry++;
			//	}
			//	if (m_nRetry > 3)    //失败超过3次 3分钟重试一次
			//	{
			//		if (diff >= 180)
			//		{
			//			Login();
			//			m_lastMsgTime.update();
			//		}
			//	}
			//	isFirst = false;
			//}
			//else
			//{
			//	if (CMMConfig::instance()->m_bUpdate)//重新登录后要判断是否有更新
			//	{
			//		std::map<CData, std::list<TSemaphore>> mapSem;
			//		ReportDevConf(); //配置更新 
			//		if(CMMParam::instance()->m_UpdateEnable == "true")
			//			NotifySendData(mapSem); //实时数据更新
			//		CMMConfig::instance()->m_bUpdate = false;
			//	}
			//	ReportAlarms();
			//	m_nRetry = 0;
			//	if (diff >= ( CMMParam::instance()->m_LoginPeriod.convertInt() * 3)) //超过三倍时间没更新时间 说明连接有问题 重新注册
			//	{
			//		SetLoginState(false);
			//		Login();
			//	}
			//}
			ReportDevConf(); 
			Poco::Thread::sleep(3000);
		}
		stop();
	}
	
	bool CMMAccess::Init()
	{		
		if (!CMMParam::instance()->Init())
		{
			return false;
		}
		CMMConfig::instance()->Init();

		SetPowerdownAlarmParam(10);		

		int webPort = CMMParam::instance()->m_WebPort.convertInt();
		int udpPort = CMMParam::instance()->m_ScDoorTransPort.convertInt();

		m_recoverPowerdownAlarmParamTimer = new ISFIT::CTimer(this, &CMMAccess::SetPowerdownAlarmParam, 1000 * 10, false, 0);
		m_updateDevTimer = new ISFIT::CTimer(this, &CMMAccess::UpdateDevConf, 1000 * 60, true, 0);
		m_wirteMeasurementFileTimer = new ISFIT::CTimer(this, &CMMAccess::WriteMeasureFile, 1000 * 60, true, 0);
		m_rebootTimer = new ISFIT::CTimer(this, &CMMAccess::AutoReboot, 1000 * 10 , true, 0, "AutoReboot");


		m_webServer->Start(webPort);            //web服务
		m_doorServer->Start(CMMParam::instance()->m_ScProtocol.c_str(), udpPort); //门禁服务
		m_server->Start(CMMParam::instance()->m_FsuEndPoint);
		m_uartService->Start();

		return true;
	}

	void CMMAccess::OnHeartBeat()
	{
		m_lastMsgTime.update();
	}

	void CMMAccess::Login()
	{
	
		//AddRoute(CMMConfig::instance()->m_SCDoorIp,CMMConfig::instance()->m_SCDoorIpRoute);
		CData loginInfo = CMMProtocolEncode::BuildLogMsg();
		if (CMMParam::instance()->m_SoapEnable == "true")
		{
			loginInfo = CMMSoapXmlEncode::setSoapSerialization(loginInfo.c_str(), 1);
		}
		CData auth_header,token;
		UpdateAuthHeader(loginInfo, auth_header, token);
		Poco::FastMutex::ScopedLock lock(m_mutex);
		HTTPResponse response;
		CData responseData;
		int nRet = m_client->SendXmlData(CMMParam::instance()->m_ScEndPoint.c_str(), loginInfo, auth_header, response, responseData);
		if (nRet < 0)
		{
			if (nRet == -3)
			{
				m_nRetry = 5; //超时 直接置为大于3值 300s重试
			}
			SetLoginState(false);
			return;
		}	
		if(nRet != 200)
		{			
			SetLoginState(false);
			return ;
		}
		if (CMMParam::instance()->m_SoapEnable == "true")
		{
			responseData = CMMSoapXmlEncode::soapClientResponseDeserialization((char*)responseData.c_str());
		}
		int ret = m_msgProcess.OnMsgProcess((char*)responseData.c_str(), NULL, 0);
		if(ret == 1)
		{
			static bool bFirstReadDev=true;
			if(bFirstReadDev)////第一次登录后读一次列表，确保设备全部读取完整
			{
				bFirstReadDev=false;
				CMMConfig::instance()->OnUpdateCfgFileTimer();
				CMMConfig::instance()->WriteMeasurefile();	
			}
			SetLoginState(true);
			m_RightLevel = ret;
			LogInfo("====Login ok ===");
			m_lastMsgTime.update();
		}
		else if (ret == 3) //token 或 认证出错
		{
			SetLoginState(false);
			LogError("cmm login failed verify to Authorization :" << ret);
		}
		else
		{
			SetLoginState(false);
			LogError("cmm login faild right level:"<<ret);
		}		
	}

	int CMMAccess::DoMsgProcess( char* request, char* response, int size )
	{
		memset(response,0,strlen(response));
		return m_msgProcess.OnMsgProcess(request, response, size);
	}

	int CMMAccess::DoMsgProcess_Error(char* request, char* response, int size, int nType, std::string errMsg)
	{
		memset(response, 0, strlen(response));
		return m_msgProcess.OnMsgProcess_Error(request, response, size, nType, errMsg);
	}

	void CMMAccess::ReportDevConf()
	{
		CData devInfo = CMMProtocolEncode::ReportDevConf();
		SendRequestToServer(devInfo);
	}

	void CMMAccess::ReportAlarms()
	{
		Poco::FastMutex::ScopedLock lock(m_alarmMutex);
		if(m_alarmList.size() == 0)
		{
			return ;
		}
		if (CMMParam::instance()->m_EnginState == "true")  //开启工程模式 不上报告警
		{
			return ;
		}
		CData reportInfo = CMMProtocolEncode::BuildAlarmReportInfo(m_alarmList);
		if(SendRequestToServer(reportInfo) == 0)
		{
			std::list<TAlarm>::iterator pos = m_alarmList.begin();
			while(pos != m_alarmList.end())
			{
				m_alarmList.erase(pos++);				
			}
		}
	}

	void CMMAccess::RemoveAlarms()
	{
		Poco::FastMutex::ScopedLock lock(m_alarmMutex);
		if(m_alarmList.size() == 0)
		{
			return ;
		}
		m_alarmList.clear();
	}

	void CMMAccess::ReportData(std::map<CData, std::list<TSemaphore>> &mapSem)
	{
		CData reportInfo = CMMProtocolEncode::BuildDataReport(mapSem);
		SendRequestToServer(reportInfo);
	}

	int CMMAccess::FromAlarmInfoToTAlarm2(std::map<CData, CData>& msg, TAlarm& alarm )
	{
		//std::vector<int> IgAlarmLevelVec = CMMParam::instance()->GetIgnoreAlarmLevel();
		//int iAlarmLevel=msg["alarmLevel"].convertInt();
		//if(std::find(IgAlarmLevelVec.begin(),IgAlarmLevelVec.end(),iAlarmLevel)!=IgAlarmLevelVec.end())
		//{
		//	//查找到要过滤的
		//	LogInfo("~~~~~~~~~~~~find IgnoreAlarmLevel, meterID:" << msg["meterId"]<<" AlarmLevel:"<<msg["alarmLevel"]<<" Config:"<<CMMConfig::instance()->m_IgnoreAlarmLevel);
		//	return -1;
		//}	

		//key: serialNO devId meterId alarmLevel alarmFlag(begin,end) describe time triggerVal
		CData serialNO= msg["serialNO"];
		CData devId= msg["devId"];
		CData meterId= msg["meterId"];
		CData alarmLevel= msg["alarmLevel"];
		CData alarmFlag= msg["alarmFlag"];
		if (alarmFlag.compare("0") == 0 || alarmFlag.compare("begin") ==0)
			alarmFlag = "BEGIN";
		else if (alarmFlag.compare("1") == 0 || alarmFlag.compare("end") == 0)
			alarmFlag = "END";
		CData describe= msg["describe"];
		CData time= msg["time"];
		CData triggerVal= msg["triggerVal"];

		int len=meterId.length();
		alarm.ID = meterId.substr(0, len - 3);
		alarm.SignalNumber = meterId.substr(len - 3, 3).convertInt();
		LogInfo("=======>> CMMAccess ====> NotifyAlarm  meterId:"<<meterId);
		if(alarm.ID.length() == 0)
		{
			return -1;
		}

		CData aliasDevId;
		std::map<CData,CData> paramMap;
		if (APPAPI::GetDevParam(devId, "msj",paramMap)>=0)
		{
			aliasDevId= paramMap["aliasDevId"];
		}


		if(aliasDevId.size()==0)
		{
			LogInfo("=======>> CMMAccess ====> aliasDevId is NULL. ");
			return -1;
		}
		
		alarm.DeviceID = aliasDevId;
		
		alarm.SerialNo =serialNO;
		

		alarm.NMAlarmID = CMMConfig::instance()->NMAlarmID(alarm.ID);
		
		alarm.AlarmTime = time;		
		alarm.AlarmFlag = alarmFlag;
		alarm.AlarmLevel = alarmLevel.convertInt();	
		CData trigger = triggerVal;
		//alarm.AlarmDesc = describe+trigger;
		alarm.AlarmDesc = describe;
		alarm.EventValue = trigger.convertDouble();
		
		CData id=meterId.substr(0,3);
		int iID=id.convertInt();

		if(len>3&&Is_rangeAlarm(iID))
		{	
		//	alarm.AlarmRemark1 = meterId;//暂不填写
			//alarm.AlarmRemark2 = triggerVal;
		}
		else
		{
			LogInfo("error alarm meterId:"<<meterId);
			return -1;
		}
		
		return 0;
	}

	int CMMAccess::GetServerStatus( TServerStatus& sts )
	{
		sts.Status = ((m_registerStatus == CMM_REGISTER_SUCCESS)?"Register Success":"Register Failed");
		sts.lastHeartBeatTime = ISFIT::timeToString(m_lastMsgTime.epochTime());
		sts.RightLevel = m_RightLevel;
		return 0;
	}

	int CMMAccess::SendRequestToServer(CData& reportInfo)
	{
		Poco::FastMutex::ScopedLock lock(m_mutex);

		if (m_registerStatus != CMM_REGISTER_SUCCESS)
		{
			return -1;
		}
		if (CMMParam::instance()->m_SoapEnable == "true")
		{
			reportInfo = CMMSoapXmlEncode::setSoapSerialization(reportInfo.c_str(), 1);
		}
		CData auth_header, token, responseData;
		HTTPResponse response;
		UpdateAuthHeader(reportInfo, auth_header, token);
		int nRet = m_client->SendXmlData(CMMParam::instance()->m_ScEndPoint.c_str(), reportInfo, auth_header, response, responseData);
		if (nRet < 0)
		{
			LogError("SendXmlData error.");
			return -1;
		}
		if (nRet != 200)
		{

			LogError("send service recv code:" << nRet << " reason:" << response.getReason().c_str());
			return -1;
		}
		if (CMMParam::instance()->m_SoapEnable == "true")
		{
			responseData = CMMSoapXmlEncode::soapClientResponseDeserialization((char*)responseData.c_str());
		}
		ISFIT::CXmlDoc doc;
		if (doc.Parse(responseData.c_str()) < 0)
		{
			return -1;
		}
		ISFIT::CXmlElement Element = doc.GetElement(CMM::Response);
		ISFIT::CXmlElement Info = Element.GetSubElement(CMM::Info);
		ISFIT::CXmlElement Type = Element.GetSubElement(CMM::PK_Type);
		int result = Info.GetSubElement("Result").GetElementText().convertInt();
		CData name = Type.GetSubElement("Name").GetElementText();
		if (result == 1)
		{
			return 0;
		}
		else
		{
			LogNotice(" name: " << name.c_str() << " return " << result);
			return -1;
		}
	}

	void CMMAccess::UpdateDevConf(int arg)
	{
		static int devConfCount = 1;
		if(CMMConfig::instance()->OnUpdateCfgFileTimer()) //如果有更新 立马更新
		{
			//ReportDevConf();
			CMMConfig::instance()->m_bUpdate = true;	
			CMMConfig::instance()->m_bUpdateBak = true;
		}
		else    //如果没更新 按设置的更新频率更新
		{
			if (devConfCount == CMMParam::instance()->m_SendPeriod.convertInt())
			{
				CMMConfig::instance()->m_bUpdate = true;	
				CMMConfig::instance()->m_bUpdateBak = true;
				devConfCount = 1;
			}
			devConfCount++;
		}
	}

	Poco::Timestamp parseTimestamp(const std::string& timestampStr) 
	{
		std::tm tmStruct = {};
		int year, month, day, hour, minute;
		
		if (sscanf(timestampStr.c_str(), "%04d%02d%02d%02d%02d", &year, &month, &day, &hour, &minute) == 5) 
		{
			tmStruct.tm_year = year - 1900; // tm_year 是从1900年开始计数的  
			tmStruct.tm_mon = month - 1;     // tm_mon 是从0开始的  
			tmStruct.tm_mday = day;
			tmStruct.tm_hour = hour;
			tmStruct.tm_min = minute;
			tmStruct.tm_sec = 0; // 秒数默认为0  
			tmStruct.tm_isdst = -1; // 让mktime决定是否为夏令时  

			// 将tm结构体转换为time_t（Unix时间戳）  
			time_t t = mktime(&tmStruct);
			return Poco::Timestamp::fromEpochTime(t);
		}
		// 解析失败，返回默认或错误的Poco::Timestamp  
		return Poco::Timestamp();
	}

	// 将文件名中的时间戳解析为 Poco::Timestamp  
	Poco::Timestamp parseTimestampFromFileName(const std::string& fileName)
	{
		size_t separator1 = fileName.find("_");  // 找到第一个下划线的位置
		if (separator1 != std::string::npos)
		{
			size_t separator2 = fileName.find("_", separator1 + 1);  // 找到第二个下划线的位置
			if (separator2 != std::string::npos)
			{
				CData datetimeStr = fileName.substr(separator2 + 1, 12);  // 提取日期时间部分
				int year = datetimeStr.substr(0, 4).convertInt();
				int month = datetimeStr.substr(4, 2).convertInt();
				int day = datetimeStr.substr(6, 2).convertInt();
				int hour = datetimeStr.substr(8, 2).convertInt();
				int minute = datetimeStr.substr(10, 2).convertInt();
				// 现在你可以使用year, month, day, hour, minute进行进一步操作
				//LogInfo("file:" << fileName << " hour:" << hour << " minute:" << minute);
				std::tm tmStruct = {};
				tmStruct.tm_year = year - 1900; // tm_year 是从1900年开始计数的  
				tmStruct.tm_mon = month - 1;     // tm_mon 是从0开始的  
				tmStruct.tm_mday = day;
				tmStruct.tm_hour = hour;
				tmStruct.tm_min = minute;
				tmStruct.tm_sec = 0; // 秒数默认为0  
				tmStruct.tm_isdst = -1; // 让mktime决定是否为夏令时  
				time_t t = mktime(&tmStruct);
				return Poco::Timestamp::fromEpochTime(t);
			}
		}
		throw std::invalid_argument("Invalid file name format: " + fileName);
	}

	// 删除指定目录下超过三天的文件  
	void deleteOldFiles(int days) 
	{
		Poco::File dir("/Measurement");
		if (dir.exists() && dir.isDirectory())
		{
			// 获取当前UTC时间戳
			time_t nowInSeconds = std::time(nullptr);
			// 计算三天前的时间戳（以秒为单位）
			time_t threeDaysAgoInSeconds = nowInSeconds - (days * 24 * 60 * 60); // 减去days天
			// 将时间戳转换为Poco::Timestamp对象
			Poco::Timestamp threeDaysAgo = Poco::Timestamp::fromEpochTime(threeDaysAgoInSeconds);
			Poco::DirectoryIterator it(dir.path());
			Poco::DirectoryIterator end;
			Poco::Path filePath;
			while (it != end)
			{
				if (it->isFile())
				{
					try 
					{
						filePath.assign(it->path());
						Poco::Timestamp fileTimestamp = parseTimestampFromFileName(filePath.getBaseName());
						// 尝试从文件名中解析时间戳
						if (fileTimestamp < threeDaysAgo && fileTimestamp!=0)
						{
							Poco::File oldFile(it->path());
							if (oldFile.exists()) 
							{
								oldFile.remove();
								LogInfo("Deleted: "<< it->path());
							}
						}
					}
					catch (const std::exception& e) 
					{
						LogError("Error processing file: " << filePath.getBaseName() << ", " << e.what());
					}
				}
				it++;
			}
		}
	}

	void CMMAccess::WriteMeasureFile(int arg)
	{
		static int nCount = 1;
		if(nCount >= CMMParam::instance()->m_CsvMeasurementTime.convertInt())   //AI历史存储周期(分钟):
		{
			deleteOldFiles(CMMParam::instance()->m_CsvExpire.convertInt());  //历史记录过期时间(天) 
			if (CMMConfig::instance()->WriteMeasurefile())
			{
				//NotifySendData(mapSem); //写监控文件后发送SEND_DATA同步
				nCount = 0;
			}		
		}
		nCount++;
	}
	
	void CMMAccess::SetPowerdownAlarmParam(int arg)
	{
		//power down alarm
		std::map<CData, CData> paramMap;
		paramMap["alarmDlyTime"] = CData(arg);
		paramMap["alarmClearDlyTime"] = CData(arg);
		APPAPI::SetMeterParam("111001", "118336001","msj", paramMap);
	}

	void CMMAccess::NotifyAlarm(std::map<CData, CData>& msg)
	{	
		std::list<CData> devIdList;
		APPAPI::GetDevId("msj", devIdList);
		//LogInfo("=======>> CMMAccess ====> NotifyAlarm== devIdList size:"<<devIdList.size());
		if(msg.size()>0)		
		{	
			CData seq; 
			{
				auto it=msg.find("serialNO");
				if (it!=msg.end()) seq = it->second;
			}
			//LogInfo("=======>> CMMAccess ====> NotifyAlarm== seq:"<<seq);
			TAlarm alarm = { };
			if(FromAlarmInfoToTAlarm2(msg, alarm) == 0)
			{		
				Poco::FastMutex::ScopedLock lock(m_alarmMutex);
				std::list<TAlarm>::iterator pos = m_alarmList.begin();
				while(pos != m_alarmList.end())
				{
					if(pos->SerialNo == seq)
					{
						if(pos->retryTimes == 0)
						{
							m_log.log(alarm);
							m_alarmList.erase(pos);
							//LogInfo("=======>> CMMAccess ====>m_alarmList.erase seq:"<<seq);
							return;
						}
					}
					pos++;
				}
				m_alarmList.push_back(alarm);
				m_alarmListBak = m_alarmList;
				m_log.log(alarm);
				//LogInfo("=======m_alarmList.push_back:"<<seq);
			}
		}
	}

	void CMMAccess::NotifySendData(std::map<CData, std::list<TSemaphore> >& mapSem)
	{
		if (mapSem.size() == 0)
		{
			CMMConfig::instance()->GetSemaphoreConf(mapSem);
		}
		ReportData(mapSem);
	}

	void CMMAccess::SetAlarmDlyTime(CData dlyTime1, CData clearDlyTime1,
										CData dlyTime2, CData clearDlyTime2,
										CData dlyTime3, CData clearDlyTime3)
	{
		std::list<CData> devIdList;
		APPAPI::GetDevId("msj", devIdList,2000);
		//LogInfo("SetAlarmDlyTime() dlyTime:"<<dlyTime<<" clearDlyTime:"<<clearDlyTime);
		for (auto it=devIdList.begin(); it!=devIdList.end(); it++)
		{
			CData devId = *it;
			//std::list<std::map<CData,CData> > valList;
			//APPAPI::GetMeterVal(devId, "msj", valList, 10000);

			std::multimap<CData,CData> attrCondition;
			attrCondition.insert(std::pair<CData,CData>("meterId",""));
			//attrCondition.insert(std::pair<CData,CData>("meterType","DI"));
			attrCondition.insert(std::pair<CData,CData>("alarmLevel","1"));
			attrCondition.insert(std::pair<CData,CData>("alarmLevel","2"));
			attrCondition.insert(std::pair<CData,CData>("alarmLevel","3"));

			std::list<std::map<CData,CData> > infoList;
			// APPAPI::GetMeterInfo(devId, "msj","or",attrCondition, infoList, 10000);
			// DAHAI 未发现调用，暂时不管

			LogInfo("devId:"<<devId);
			
			std::list<std::map<CData,CData> > paramList;
			for (auto vit=infoList.begin(); vit!=infoList.end(); vit++)
			{
				std::map<CData,CData>& attr = *vit;
				for (auto ait=attr.begin(); ait!=attr.end(); ait++)
				{
					//LogInfo("==key:"<<ait->first<<" val:"<<ait->second);	
				}
				
				CData meterId = attr["meterId"];
				CData meterType = attr["meterType"];
				CData alarmLevel = attr["alarmLevel"];
				CData id=meterId.substr(0,3);
				//int iID=id.convertInt();

				//LogInfo("meterId:"<<meterId<<" meterType:"<<meterType<<" alarmLevel:"<<alarmLevel);
				if (meterId.size()>3 && (alarmLevel=="1"))
				{
					std::map<CData,CData> attrMap;
					attrMap["meterId"] = meterId;
					attrMap["alarmDlyTime"] = dlyTime1;
					attrMap["alarmClearDlyTime"] = clearDlyTime1;
					paramList.push_back(attrMap);
				}
				else if (meterId.size()>3 && (alarmLevel=="2"))
				{
					std::map<CData,CData> attrMap;
					attrMap["meterId"] = meterId;
					attrMap["alarmDlyTime"] = dlyTime2;
					attrMap["alarmClearDlyTime"] = clearDlyTime2;
					paramList.push_back(attrMap);
				}
				else if (meterId.size()>3 && (alarmLevel=="3"))
				{
					std::map<CData,CData> attrMap;
					attrMap["meterId"] = meterId;
					attrMap["alarmDlyTime"] = dlyTime3;
					attrMap["alarmClearDlyTime"] = clearDlyTime3;
					paramList.push_back(attrMap);
				}
			}
			std::list<CData> errorMeterIdList;
			APPAPI::SetMeterParam(devId, "msj", paramList, errorMeterIdList,0);
			
		}		
	}
	
	void CMMAccess::initialize(std::list<std::tuple<CData, CData> >& param)
	{
		CData appDataDir = "/appdata";
		CData userDataDir = "/userdata";

		CData userPicDir=userDataDir+"/PIC";
		Poco::File userPicPath(userPicDir.c_str());
		if (userPicPath.exists() == false)
		{
			ISFIT::Shell("mkdir -m 777 " + userPicDir);
			LogInfo("======userPicPath:"<<userPicDir<<" not exists, create it===");
		}
		
		Poco::File picLnDir("/PIC");
		if (picLnDir.exists() == false)
		{
			ISFIT::Shell("ln -s " + userPicDir + "  /PIC");
			LogInfo("======/PIC not exists, create it====");
		}
		
		//真实创建
		CData usercmmDir=userDataDir+"/Config";
		Poco::File usercmmPath(usercmmDir.c_str());
		if (usercmmPath.exists() == false)
		{
			ISFIT::Shell("mkdir -m 777 " + usercmmDir);
			LogInfo("======usercmmPath:"<<usercmmDir<<" not exists, create it===");
		}

		Poco::File cmmLnDir("/Config");
	/*		if (cmmLnDir.exists() ==true)
		{
			ISFIT::Shell("rm -rf  /Config");
			LogInfo("======/Config not exists, create it====");
		}*/
		
		if (cmmLnDir.exists() == false)
		{
			ISFIT::Shell("ln -s " + usercmmDir + "  /Config");
			LogInfo("======/Config not exists, create it====");
		}
		
		//软链接
		CData userAlarmDir=userDataDir+"/Alarm";
		Poco::File userAlarmPath(userAlarmDir.c_str());
		if (userAlarmPath.exists() == false)
		{
			ISFIT::Shell("mkdir -m 777 " + userAlarmDir);
			LogInfo("======userAlarmPath:"<<userAlarmDir<<" not exists, create it===");
		}
		
		Poco::File alarmLnDir("/Alarm");
		if (alarmLnDir.exists() == false)
		{
			ISFIT::Shell("ln -s " + userAlarmDir + "  /Alarm");
			LogInfo("======/Alarm not exists, create it====");
		}
		
	//软链接
		Poco::File logsLnDir("/logs");
		if (logsLnDir.exists() == false)
		{
			ISFIT::Shell("ln -s /userdata/log/  /logs");
			LogInfo("======/logs not exists, create it====");
		}
		
		//软链接
		CData userMeasureDir=userDataDir+"/Measurement";
		Poco::File userMeasurePath(userMeasureDir.c_str());
		if (userMeasurePath.exists() == false)
		{
			ISFIT::Shell("mkdir -m 777 " + userMeasureDir);
			LogInfo("======userMeasurePath:"<<userMeasureDir<<" not exists, create it===");
		}
		
		Poco::File measureLnDir("/Measurement");
		if (measureLnDir.exists() == false)
		{
			ISFIT::Shell("ln -s " + userMeasureDir + "  /Measurement");
			LogInfo("======/Measurement not exists, create it====");
		}
		
		//软链接
		CData upgradeDir=userDataDir+"/upgrade";
		Poco::File upgradePath(upgradeDir.c_str());
		if (upgradePath.exists() == false)
		{
			ISFIT::Shell("mkdir -m 777 " + upgradeDir);
			LogInfo("======upgradePath:"<<upgradeDir<<" not exists, create it===");
		}
		
		Poco::File upgradeLnDir("/upgrade");
		if (upgradeLnDir.exists() == false)
		{
			ISFIT::Shell("ln -s " + upgradeDir + "  /upgrade");
			LogInfo("======/upgrade not exists, create it====");
			
		}
		
		param.push_back(std::make_tuple(CData(CMM::param::LoginState), CData("未注册")));
		param.push_back(std::make_tuple(CData(CMM::param::DoorLoginState), CData("未注册")));

		/*param.push_back(std::make_tuple(CData(CMM::param::AlarmSendDB), CData("/userdata/db/alarm-sent-status.sqlite")));
		param.push_back(std::make_tuple(CData(CMM::param::FsuDeviceId), CData("")));

		param.push_back(std::make_tuple(CData(CMM::param::CsvEncoding), CData("GB18030//TRANSLIT")));*/
		param.push_back(std::make_tuple(CData(CMM::param::CsvExpire), CData("15")));
		param.push_back(std::make_tuple(CData(CMM::param::CsvMeasurementTime), CData("30")));
		param.push_back(std::make_tuple(CData(CMM::param::SCDoorIp), CData("192.168.1.191")));
		param.push_back(std::make_tuple(CData(CMM::param::SCDoorPort), CData("11216")));
		param.push_back(std::make_tuple(CData(CMM::param::SCDoorTransPort), CData("11215")));
		param.push_back(std::make_tuple(CData(CMM::param::SCProtocol), CData("udp")));

		param.push_back(std::make_tuple(CData(CMM::param::AuthEnable), CData("false")));
		param.push_back(std::make_tuple(CData(CMM::param::EnginState), CData("false")));
		param.push_back(std::make_tuple(CData(CMM::param::UpdateEnable), CData("false")));
		param.push_back(std::make_tuple(CData(CMM::param::SoapEnable), CData("false")));

		param.push_back(std::make_tuple(CData(CMM::param::RebootEnable), CData("false")));
		param.push_back(std::make_tuple(CData(CMM::param::RebootPeriod), CData("7")));
		param.push_back(std::make_tuple(CData(CMM::param::RebootTime), CData("22:00:00")));
		param.push_back(std::make_tuple(CData(CMM::param::FsuId), CData("33202412230008")));
		param.push_back(std::make_tuple(CData(CMM::param::FsuEndPoint), CData("http://192.168.1.191:8080/v1/services/newFSUService")));

		param.push_back(std::make_tuple(CData(CMM::param::FtpUsr), CData("sftp")));
		param.push_back(std::make_tuple(CData(CMM::param::FtpPasswd), CData("qwer@123")));
		param.push_back(std::make_tuple(CData(CMM::param::FtpType), CData("sftp")));

		param.push_back(std::make_tuple(CData(CMM::param::LogFileSize), CData("1")));
		param.push_back(std::make_tuple(CData(CMM::param::LogLevel), CData("6")));
		param.push_back(std::make_tuple(CData(CMM::param::LoggerCount), CData("2")));

		param.push_back(std::make_tuple(CData(CMM::param::LoginPeriod), CData("30")));
		param.push_back(std::make_tuple(CData(CMM::param::Algorithm), CData("sha256")));
		param.push_back(std::make_tuple(CData(CMM::param::Password), CData("admin")));
		param.push_back(std::make_tuple(CData(CMM::param::UserName), CData("password")));
		param.push_back(std::make_tuple(CData(CMM::param::SCEndPoint), CData("http://192.168.1.191:8080/v1/services/newLSCService")));


		param.push_back(std::make_tuple(CData(CMM::param::SendPeriod), CData("30")));
		param.push_back(std::make_tuple(CData(CMM::param::UpdateInterval), CData("120")));

		param.push_back(std::make_tuple(CData(CMM::param::WebDeviceConfig), CData("/appdata/config/MobileBConfig.json")));
		//param.push_back(std::make_tuple(CData(CMM::param::WebQueueDepth), CData("1024")));
		param.push_back(std::make_tuple(CData(CMM::param::WebInterval), CData("120")));
		param.push_back(std::make_tuple(CData(CMM::param::WebHost), CData("0.0.0.0")));
		/*param.push_back(std::make_tuple(CData(CMM::param::WebHtdocs), CData("/userdata/htdocs")));
		param.push_back(std::make_tuple(CData(CMM::param::WebMimes), CData("/userdata/conf/mime.types.properties")));*/
		param.push_back(std::make_tuple(CData(CMM::param::WebPort), CData("9080")));
		//param.push_back(std::make_tuple(CData(CMM::param::WebStorageDevice), CData("/dev/mmcblk0p11")));
		param.push_back(std::make_tuple(CData(CMM::param::WebVersion), CData("WebAPI/1.0")));

		param.push_back(std::make_tuple(CData(CMM::param::SiteID), CData("5101072000001")));
		param.push_back(std::make_tuple(CData(CMM::param::SiteName), CData("川成都高升桥枢纽站")));
		param.push_back(std::make_tuple(CData(CMM::param::RoomID), CData("000000011")));
		param.push_back(std::make_tuple(CData(CMM::param::RoomName), CData("室内汇聚机房")));

		param.push_back(std::make_tuple(CData(CMM::param::UartName), CData("COM1")));
		param.push_back(std::make_tuple(CData(CMM::param::BaudRate), CData("9600")));
		param.push_back(std::make_tuple(CData(CMM::param::DataBit), CData("8")));
		param.push_back(std::make_tuple(CData(CMM::param::Parity), CData("None")));
		param.push_back(std::make_tuple(CData(CMM::param::StopBit), CData("1")));
		param.push_back(std::make_tuple(CData(CMM::param::SlaveID), CData("1")));
		
		/*param.push_back(std::make_tuple(CData(CMM::param::FlowControl), CData("N")));
		param.push_back(std::make_tuple(CData(CMM::param::LoggerChannel), CData("file")));*/
		LogInfo("=====finish init module name:---------");
		
	}

	void CMMAccess::start()
	{			
		LogInfo("======>CMMAccess start====>");
		if(m_bStart)
		{
			LogError("CMMAccess has started。");
			return ;
		}
		m_bStart = true;
		if (!Init())
		{
			LogError("CMMAccess initial failed.");
			return;
		}
		m_thread.start(*this);
		m_secondThread.start(threadFunction2, this);
		LogInfo("======>CMMAccess start ok====>");
	}

	void CMMAccess::stop()
	{	
		LogInfo("=== CMMAccess go to stop  ====");	
		m_bStart = false;
		DeInit();
		m_thread.join();
		m_secondThread.join();
		SetLoginState(false);
		SetUdpLoginState(false);
		LogInfo("=== CMMAccess stop ok ====");	
	}
	
	void CMMAccess::DeInit()
	{
		if(m_doorServer)
			m_doorServer->stopServer();
		if(m_server)
			m_server->Stop();	
		if (m_uartService)
			m_uartService->Stop();
		if(m_webServer)
			m_webServer->Stop();
	}

	void CMMAccess::unInitialize()
	{
	}

	/*
	* 检查是否是白名单用户
	*/
	bool CMMAccess::checkIpAuth(const char* http_ip, int familyType)
	{
		CData strFamilyType;
		if (familyType == 2)
			strFamilyType = "IPV4";
		if (familyType == 23)
			strFamilyType = "IPV6";
		if (!CMMConfig::instance()->isAcceptIp(strFamilyType, http_ip))
		{
			LogNotice("client ip: " << http_ip << " is no access.");
			return false;
		}
		LogNotice("client ip: " << http_ip << " family type:" << strFamilyType.c_str());
		return true;
	}


	/*
	* 检查服务端token和计算的token是否一致
	*/
	bool CMMAccess::checkAuth(const char* http_auth_header, char* _xmlData)
	{
		LogInfo("recv checkAuth = " << http_auth_header << "\r\n");
		CData token,strToken;
		CData strOutMsg;
		CData strInMsg(_xmlData);
		UpdateAuthHeader(strInMsg,strOutMsg, token);

		std::stringstream ss(http_auth_header);
		std::string line, key, value;
		while (std::getline(ss, line, '\n'))
		{
			if (line.find("Authorization:") != std::string::npos)
			{
				Poco::RegularExpression regex(R"~(token="([^"]*)")~");
				Poco::RegularExpression::MatchVec matches;
				if (regex.match(line, 0, matches))
				{
					strToken = line.substr(matches[1].offset, matches[1].length);
				}
			}
		}

		LogInfo("recv token = " << strToken.c_str() << "\n calculate token = " << token.c_str());
		if (strToken.compare(token) != 0)
			return false;
		return true;
	}

	void CMMAccess::UpdateAuthHeader(const CData& message, CData& authHeader, CData& strtoken)
	{
		CData strUser = CMMParam::instance()->m_UserName;
		CData strPass = CMMParam::instance()->m_Password;
		//CData strVersion = CMMConfig::instance()->m_fsuVersion;
		CData strOutPass;
		//if (true == CTextEncryption::hashMessage(strPass, strOutPass, 1)) //sha256 散列passwd
		{
			//LogInfo("hashMessage passwd = " << strOutPass.c_str());
			strtoken = CTextEncryption::getToken(strPass, message);  //2. 原始passwd和 进行HMAC-SHA-256计算 再转成Base16
			authHeader = "appid=\"" + strUser + "\",token=\"" + strtoken + "\",v=\"4.5.0\"";
		}
	}

	CData CMMAccess::resolveDomainToIp(const char* domainName) 
	{
		LogInfo("resolveDomainToIp : " << domainName);
		if (strlen(domainName) < 1)
			return "";
		CData domainIp;
		struct addrinfo hints, * res, * p;
		int status;

		// 清空 hints 结构体
		memset(&hints, 0, sizeof(hints));

		// 设置 hints 参数
		hints.ai_family = AF_UNSPEC; // 可以是 IPv4 或 IPv6
		hints.ai_socktype = SOCK_STREAM; // 指定套接字类型，此处为流式套接字
		hints.ai_protocol = IPPROTO_TCP; // 可选，如果你关心协议的话

		// 执行域名解析
		if ((status = getaddrinfo(domainName, nullptr, &hints, &res)) != 0) 
		{
			LogInfo("getaddrinfo error: " << gai_strerror(status));
			return "";
		}

		// 遍历结果链表并打印IP地址
		for (p = res; p != nullptr; p = p->ai_next) 
		{
			char ipStringBuffer[INET_ADDRSTRLEN]; // 缓冲区足够容纳IPv4或IPv6地址
			void* addressPtr = nullptr;
			// 根据地址族选择适当的指针
			if (p->ai_family == AF_INET) 
			{
				sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
				addressPtr = &(ipv4->sin_addr);
				inet_ntop(p->ai_family, addressPtr, ipStringBuffer, sizeof(ipStringBuffer));
				domainIp = ipStringBuffer;
				LogInfo("IP Address: " << ipStringBuffer);
				break;
			}
			/*else if (p->ai_family == AF_INET6) {
				sockaddr_in6* ipv6 = reinterpret_cast<sockaddr_in6*>(p->ai_addr);
				addressPtr = &(ipv6->sin6_addr);
			}*/	
		}
		// 释放getaddrinfo返回的资源
		freeaddrinfo(res);
		return domainIp;
	}

	bool CMMAccess::SendUartDataToSC(std::vector<uint8_t>& sendBuffer)
	{
		if (0 > m_doorClient->SendData(CMMParam::instance()->m_ScUdpPoint.c_str(), CMMParam::instance()->m_ScProtocol,sendBuffer))
			return false;
		return true;
	}

	bool CMMAccess::writeDataToUart(std::vector<uint8_t>& sendBuffer)
	{
		if (!m_uartService->writeData(sendBuffer))
			return false;
		return true;
	}

	void CMMAccess::SaveDataLog(std::map<CData, CData>& msg)
	{
		m_datalog.Save(msg);
	}

	
	void CMMAccess::AutoReboot(int arg)
	{
		if (CMMParam::instance()->m_RebootEnable == "false") 
		{
            return; // 如果未启用定时重启，直接返回
        }

		Poco::LocalDateTime now;
		LogInfo("Local Time: " << Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
        // 解析用户设置的重启时间
        std::string rebootTimeStr = CMMParam::instance()->m_RebootTime.c_str();
		Poco::DateTime rebootTime;
		int tz;
		// 解析时间字符串（假设格式为 "%H:%M:%S"）
		DateTimeParser::parse("%H:%M:%S", rebootTimeStr, rebootTime, tz);
		Poco::LocalDateTime rebootLocalTime(rebootTime.timestamp());
		// 设置重启时间的日期部分为当前日期
		rebootLocalTime.assign(now.year(), now.month(), now.day(), rebootTime.hour(), rebootTime.minute(), rebootTime.second());
		LogInfo("now : " << now.year() << now.month() << now.day() << now.hour() << now.minute()<<  now.second());
		LogInfo("reboot : "<<rebootLocalTime.year()<< rebootLocalTime.month()<< rebootLocalTime.day()<<rebootLocalTime.hour()<< rebootLocalTime.minute()<< rebootLocalTime.second());
		// 如果重启时间已经过了当前时间，设置为明天的同一时间
		if (rebootLocalTime < now)
		{
			rebootLocalTime += Timespan(1, 0, 0, 0, 0); // 加一天
		}

		Timespan duration = rebootLocalTime - now;
		LogInfo("Time until next reboot check: " << duration.totalSeconds() << " seconds");
		// 如果今天的时间已经过了，设置为明天的同一时间
		// 如果当前时间等于重启时间，检查重启周期
		if (duration.totalSeconds() <= 10)
		{
			
			// 获取重启周期（天数）
			int rebootIntervalDays = CMMParam::instance()->m_RebootPeriod.convertInt();

			// 使用静态变量记录天数
			static int dayCounter = 1;
			

			// 检查是否达到重启周期
			if (dayCounter >= rebootIntervalDays)
			{
				Poco::Thread::sleep(1000 * duration.totalSeconds());  //间隔小于线程调用间隔 直接内部等待
				LogInfo("Performing reboot...");
				APPAPI::RebootSys();
				dayCounter = 1; // 重置计数器
			}
			else
			{
				dayCounter++;
				LogInfo("Reboot condition not met. Day counter: " << dayCounter);
			}
		}
	}

	void CMMAccess::setUartParam(CData key, CData value)
	{
		if (key == CMM::param::UartName)
		{
			m_uartService->setUartName(value.c_str());
		}
		else if (key == CMM::param::BaudRate)
		{
			m_uartService->setBaudrate(value.convertInt());
		}
		else if (key == CMM::param::DataBit)
		{
			m_uartService->setDataBits(value.convertInt());
		}
		else if (key == CMM::param::Parity)
		{
			m_uartService->setParity(value.c_str());
		}
		else if (key == CMM::param::StopBit)
		{
			m_uartService->setStopBits(value.convertInt());
		}
		else if (key == CMM::param::SlaveID)
		{
			m_uartService->setSlaveID(value.convertInt());
		}

	}

	void CMMAccess::setWebParam(CData key, CData value)
	{
		if (key == CMM::param::WebPort)
		{
			m_webServer->ListenPortChange(value.convertInt());
		}
	}

	void CMMAccess::setHttpParam(CData key, CData value)
	{
		if (key == CMM::param::FsuEndPoint)
		{
			m_server->ListenPortChange(value);
		}
	}

	void CMMAccess::setDoorServeParam(CData key, CData value, CData port)
	{
		if (key == CMM::param::SCProtocol) //透传协议变更
		{
			m_doorServer->Start(value.c_str(), port.convertInt());
		}
		else if(key == CMM::param::SCDoorTransPort)  //端口变更
		{
			m_doorServer->Start(value.c_str(), port.convertInt());
		}
	}

	int CMMAccess::SendGetDeviceData(std::map<std::string, std::list<std::string>>& devList)
	{
		std::string response;
		CData uri("http://127.0.0.1/jscmd/getDevLevelList");
		int nRet = m_client->SendGetDeviceData(uri.c_str(),response);
		if (nRet == 0)
		{
			CMMConfig::instance()->GetDeviceConfig()->parseDevList(response,devList);
			LogInfo("this devsize: " << devList.size());
			auto iter = devList.begin();
			for (; iter != devList.end(); ++iter)
			{
				LogDebug("main dev:" << iter->first << " size: "<< iter->second.size());
			}
		}
		return nRet;
	}
}

