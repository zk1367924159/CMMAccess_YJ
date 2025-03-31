#include "CMMMsgProcess.h"
#include "CLog.h"
#include "CMMProtocolEncode.h"
#include "CMMAccess.h"
#include "CMMProtocolDecode.h"
#include "CMMConfig.h"
#include "CMMMeteTranslate.h"
#include "SysCommon.h"
#include "../../ExtAppIpc/ExtAppIpcApi.h"
//#include "ExtSoApi.h"
//#include "../../NetComm/FSUUtil.h"

namespace CMM
{
	CData CMMMsg::GetMethod()
	{
		return m_method;
	}

	ISFIT::CXmlElement& CMMMsg::GetInfoNode()
	{
		return m_infoNode;
	}

	int CMMMsg::Prase( char *msg )
	{
		if(m_doc.Parse(msg) < 0)
		{
			return -1;
		}
		return 0;
	}

	ISFIT::CXmlDoc& CMMMsg::GetXmlDoc()
	{
		return m_doc;
	}

	int CMMMsg::Decode(ISFIT::CXmlElement&)
	{
		return -1;
	}

	int CMMMsg::SetResponseXml( CData &xml )
	{
		return -1;
	}

	CMMMsg::~CMMMsg()
	{

	}




	int CMMRequset::Decode(ISFIT::CXmlElement& element)
	{
		m_method = element.GetSubElement(CMM::PK_Type).GetSubElement(CMM::Name).GetElementText();
		m_infoNode = element.GetSubElement(CMM::Info);
		return 0;
	}


	CMMResponse::CMMResponse()
	{
		m_rspBuf = NULL;
		m_bufSize = 0;
	}

	CMMResponse::CMMResponse( char* rspBuf, int size )
	{
		m_rspBuf = rspBuf;
		m_bufSize = size;
	}

	int CMMResponse::Decode(ISFIT::CXmlElement& element)
	{
		m_method = element.GetSubElement(CMM::PK_Type).GetSubElement(CMM::Name).GetElementText();
		m_infoNode = element.GetSubElement(CMM::Info);
		return 0;
	}

	int CMMResponse::SetResponseXml( CData &xml )
	{
		if(xml.length() > m_bufSize)
		{
			LogError("cmm response data is too big");
			return -1;
		}
		strcpy(m_rspBuf, xml.c_str());
		return 0;
	}


	int MsgProcess::OnMsgProcess( char * msg, char *returnBuf, int size )
	{
		ISFIT::CXmlDoc doc;
		if(doc.Parse(msg) < 0)
		{
			return -1;
		}
		ISFIT::CXmlElement element = doc.GetElement(CMM::Request);
		if(element != NULL)
		{
			return DoRequest(element, returnBuf, size);
		}
		element = doc.GetElement(CMM::Response);
		if(element != NULL)
		{
			return DoResponse(element);
		}
		return -1;		
	}

	int MsgProcess::OnMsgProcess_Error(char* msg, char* returnBuf, int size, int enumResult,std::string errmsg)
	{
		ISFIT::CXmlDoc doc;
		if(doc.Parse(msg) < 0)
		{
			return -1;
		}
		ISFIT::CXmlElement element = doc.GetElement(CMM::Request);
		CMMResponse response(returnBuf, size);
		if (element == NULL)
		{
			LogError("this is error xml to find request: " << msg);
			CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::UNCONFIG, "SC未配置", "");
			response.SetResponseXml(rsp);
			return 0;
		}
		response.Decode(element);
		CData resAck = response.GetMethod() + "_ACK";
		if (enumResult == 0)
		{
			CData rsp;
			if (errmsg == "")
			{
				rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::FAILURE, "其他失败原因", resAck);
			}
			else
			{
				rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::FAILURE, errmsg.c_str(), resAck);
			}
			response.SetResponseXml(rsp);
		}
		else if (enumResult == 2)
		{
			CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::ILLEGALACCESS, "未授权非法访问",resAck);
			response.SetResponseXml(rsp);
		}
		else if (enumResult == 3)
		{
			CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::AUTHERROR, "报文认证失败",resAck);
			response.SetResponseXml(rsp);
		}
		else if (enumResult == 4)
		{
			CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::NODATA, "无数据",resAck);
			response.SetResponseXml(rsp);
		}

		LogInfo("=====>> response : " << returnBuf);
		return 1;
	}

	

	MsgProcess::MsgProcess()
	{
		m_msgMap[CMM::method::GET_DEV_CONF] = &MsgProcess::OnGetDevConf;
		m_msgMap[CMM::method::SET_DEV_CONF_DATA] = &MsgProcess::OnSetDevConf;
		m_msgMap[CMM::method::GET_DATA] = &MsgProcess::OnGetData;
		m_msgMap[CMM::method::SET_POINT] = &MsgProcess::OnSetPoint;
		m_msgMap[CMM::method::GET_THRESHOLD] = &MsgProcess::OnGetThreshold;
		m_msgMap[CMM::method::SET_THRESHOLD] = &MsgProcess::OnSetThreshold;
		m_msgMap[CMM::method::GET_FTP] = &MsgProcess::OnGetFtpInfo;
		m_msgMap[CMM::method::SET_FTP] = &MsgProcess::OnSetFtpInfo;
		m_msgMap[CMM::method::GET_LOGININFO] = &MsgProcess::OnGetLoginInfo;
		m_msgMap[CMM::method::SET_LOGININFO] = &MsgProcess::OnSetLoginInfo;
		m_msgMap[CMM::method::GET_STORAGERULE] = &MsgProcess::OnGetStorageRule;
		m_msgMap[CMM::method::SET_STORAGERULE] = &MsgProcess::OnSetStorageRule;

		m_msgMap[CMM::method::GET_FSUINFO] = &MsgProcess::OnGetFSUInfo;
		m_msgMap[CMM::method::UPDATE_FSUINFO_INTERVAL] = &MsgProcess::OnUpdateFsuInterval;
		m_msgMap[CMM::method::TIME_CHECK] = &MsgProcess::OnTimeCheck;
		m_msgMap[CMM::method::SET_FSUREBOOT] = &MsgProcess::OnReboot;
		m_msgMap[CMM::method::GET_TIME] = &MsgProcess::OnGetTime;
		m_msgMap[CMM::method::SET_ACCEPT_IP_CONF] = &MsgProcess::OnSetAcceptIP;
		m_msgMap[CMM::method::SET_FSUREBOOT] = &MsgProcess::OnSetFsuReboot;
		m_msgMap[CMM::method::LOGIN] = &MsgProcess::OnLogin;
	}

	int MsgProcess::DoRequest(ISFIT::CXmlElement& element, char* returnBuf, int size)
	{
		CMMRequset request;
		CMMResponse response(returnBuf, size);
		request.Decode(element);
		std::map<CData, processFun>::iterator pos = m_msgMap.find(request.GetMethod());
		if(pos == m_msgMap.end())
		{
			LogError("cmm msg can not process method name:"<<request.GetMethod());
			return -1;
		}
		ISFIT::CXmlElement info = request.GetInfoNode();
		CData method = request.GetMethod();
		CData fusId = info.GetSubElement("FSUID").GetElementText();
		int len=strlen(fusId.c_str());
		int len2=strlen(CMMParam::instance()->m_FsuId.c_str());
		if(((fusId != CMMParam::instance()->m_FsuId)
			&&(method.compareNoCase(CMM::method::SET_LOGININFO) !=0)
			&&(method.compareNoCase(CMM::method::TIME_CHECK)!=0)) || ((len==0&&len2==0)&&(method.compareNoCase(CMM::method::SET_LOGININFO) !=0)&&(method.compareNoCase(CMM::method::TIME_CHECK)!=0)))
		{			
			CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::FAILURE, "fsuid invalid", method+"_ACK");
			response.SetResponseXml(rsp);
		}
		else
		{
			processFun fun = pos->second;
			(this->*fun)(request, response);
			LogInfo("=====>> response : "<<returnBuf);
		}
		return 1;
	}

	int MsgProcess::DoResponse(ISFIT::CXmlElement& element)
	{
		CMMResponse response;
		response.Decode(element);
		if(response.GetMethod().compare(CMM::method::LOGIN_ACK) == 0){
			return OnLoginRsp(response.GetInfoNode());
		}
		return -1;
	}
		
	int MsgProcess::OnLoginRsp(ISFIT::CXmlElement& info)
	{
		return info.GetSubElement("Result").GetElementText().convertInt();
	}

	bool MsgProcess::ThresholdIdFilter(std::map<CData,CData>& attr)
	{
		CData meterId = attr["meterId"];
		if (meterId.length()<9)
		{
			return false;
		}
		CData meterType = attr["meterType"];
		CData subId = meterId.substr(0, 3);
		//LogInfo("----subId:"<<subId<<" meterType:"<<meterType);
		if (meterType == "DI")
			return true;
		return false;
	}

	int MsgProcess::OnGetDevConf( CMMMsg& request, CMMMsg & response )
	{
#if 0
	 	ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement deviceList = info.GetSubElement("DeviceList");
		std::list<CData> devList;
		int index = 0;
		ISFIT::CXmlElement device = deviceList.GetSubElement("Device", index++);
		CData deviceId;
		while(device != NULL)
		{
			deviceId = device.GetAttribute("ID");
			if(deviceId.length() == 0)
			{
				devList.clear();
				break;
			}
			devList.push_back(deviceId);
			device = deviceList.GetSubElement("Device", index++);
		}
		CData rsp = CMMProtocolEncode::GetDevConf(devList);
		response.SetResponseXml(rsp);
#else
		ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement deviceList = info.GetSubElement("DeviceID");
		std::list<CData> devList;		
		CData deviceId = deviceList.GetElementText().convertString();
		devList.push_back(deviceId);
		CData rsp = CMMProtocolEncode::GetDevConf(devList);
		response.SetResponseXml(rsp);
#endif
		return 0;
	}

	int MsgProcess::OnSetDevConf( CMMMsg& request, CMMMsg & response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		//ISFIT::CXmlElement deviceList = info.GetSubElement("Values").GetSubElement("DeviceList");
		ISFIT::CXmlElement deviceList = info.GetSubElement("Values");//无deviceList
		std::map<CData, TDevConf> devMap;
		CProtocolDecode::DecodeDevCfg(deviceList, devMap);
		
		std::list<CData> success, failed;
		CData failedCause="NULL";	
		int result=0;
		if(devMap.size()==0)
			failedCause="请求体未识别到要配置的设备id";
		else{	
			int ret=CMMConfig::instance()->SetDevCfg(devMap,success,failed);
			result=1;
			if(ret<0)
			{
				result=0;
				failedCause="未匹配上对应的设备id";
			}
		}
	/*	std::map<CData, TDevConf>::iterator pos = devMap.begin();
		while(pos != devMap.end())
		{
			success.push_back(pos->first);
			pos++;
		}*/
		CData rsp = CMMProtocolEncode::GenGeneralSetRsp(result,failedCause,CMM::method::SET_DEV_CONF_DATA_ACK, success, failed);
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnGetData( CMMMsg& request, CMMMsg & response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement deviceList = info.GetSubElement("DeviceList");
		
		std::map<CData, std::list<TSemaphore> > reqDevMap;
		CProtocolDecode::DecodeGetDeviceList(deviceList, reqDevMap);
		
		std::map<CData, std::list<TSemaphore> > rspDevMap;
		CMMConfig::instance()->GetSemaphoreConf(reqDevMap);
		bool bOK=true;
		CData rsp;
		int nResult = CMM::SUCCESS;
		if (0 == reqDevMap.size())
		{
			if (rspDevMap.size() == 0)
			{
				nResult = CMM::NODATA;
			}
			for (auto iter = rspDevMap.begin(); iter != rspDevMap.end(); ++iter)
			{
				std::list<TSemaphore>& list = iter->second;
				for (auto it = list.begin(); it != list.end(); ++it)
				{
					it->result = 1;
				}
			}
			 rsp = CMMProtocolEncode::BuildGetDataRsp(nResult, rspDevMap);
		}
		else
		{
			if (rspDevMap.size() == 0)
			{
				nResult = CMM::NODATA;
			}
			else
			{
				for (auto iter = reqDevMap.begin(); iter != reqDevMap.end(); ++iter)
				{
					CData deviceId = iter->first;
					std::list<TSemaphore>& recvConfig = iter->second;
					auto key = rspDevMap.find(deviceId);
					if (key == rspDevMap.end())
					{
						continue;
					}
					std::list<TSemaphore>& config = key->second;
					for (auto it = recvConfig.begin(); it != recvConfig.end(); ++it)
					{
						for (auto its = config.begin(); its != config.end(); ++its)
						{
							if (it->ID == its->ID)
							{
								it = its;
								it->result = 1;
							}
						}
					}
				}
			}
			rsp = CMMProtocolEncode::BuildGetDataRsp(nResult, reqDevMap);
		}
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnSetPoint( CMMMsg& request, CMMMsg & response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement value = info.GetSubElement("Value");
		ISFIT::CXmlElement deviceList = value.GetSubElement("DeviceList");
		std::map<CData, std::list<TSemaphore> > devMap;
		CProtocolDecode::DecodeSetPoint(deviceList, devMap);
		bool bOK = true;
		CData failedCause="NULL";
		if(devMap.size() == 0)
		{
			failedCause="请求体未识别到设备id";
			bOK = false;
		}
		else
		{
			auto pos = devMap.begin();
			while (pos != devMap.end())
			{
				std::list<TSemaphore> & pointList = pos->second;
				int ret = CMMConfig::instance()->SetSemaphoreConf(pos->first, pointList);
				if (ret < 0)
				{
					bOK = false;
					if (ret == -2)
						failedCause = "设置值不符合范围";
					else
						failedCause = "未查找到对应设备或id";
				}
				pos++;
			}
		}
		if (bOK)
		{
			CMMConfig::instance()->UpdateCfgFile();
		}
		CData rsp = CMMProtocolEncode::BuildSetPointRsp(bOK?CMM::SUCCESS:CMM::FAILURE,failedCause,CMM::method::SET_POINT_ACK, devMap);
		response.SetResponseXml(rsp);
		//if(devMap.size() !=0)
		//	CMMAccess::instance()->NotifySendData(devMap);  //写监控数据后 上报操作设备的监控点数据
		return 0;
	}

	int MsgProcess::OnGetThreshold( CMMMsg& request, CMMMsg & response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement deviceList = info.GetSubElement("DeviceList");

		std::map<CData, std::list<TThreshold> > reqDevMap;
		CProtocolDecode::DecodeGetDeviceList(deviceList, reqDevMap);	
	
		std::map<CData, std::list<TThreshold>> rspDevMap;
		CMMConfig::instance()->GetThresholdConf(rspDevMap);
		bool bOK=true;
		CData rsp;
		int nResult = CMM::SUCCESS;
		if (0 == reqDevMap.size())
		{
			if (rspDevMap.size() == 0)
			{
				nResult = CMM::NODATA;
			}
			for (auto iter = rspDevMap.begin(); iter != rspDevMap.end(); ++iter)
			{
				std::list<TThreshold>& list = iter->second;
				for (auto it = list.begin(); it != list.end(); ++it)
				{
					it->result = 1;
				}
			}
			 rsp = CMMProtocolEncode::BuildGetThresholdRsp(nResult, rspDevMap);
		}
		else
		{
			if (rspDevMap.size() == 0)
			{
				nResult = CMM::NODATA;
			}
			else
			{
				for (auto iter = reqDevMap.begin(); iter != reqDevMap.end(); ++iter)
				{
					CData deviceId = iter->first;
					std::list<TThreshold>& recvConfig = iter->second;
					auto key = rspDevMap.find(deviceId);
					if (key == rspDevMap.end())
					{
						continue;
					}
					std::list<TThreshold>& config = key->second;
					for (auto it = recvConfig.begin(); it != recvConfig.end(); ++it)
					{
						for (auto its = config.begin(); its != config.end(); ++its)
						{
							if (it->ID == its->ID)
							{
								it = its;
								it->result = 1;
							}
						}
					}
				}
			}
			rsp = CMMProtocolEncode::BuildGetThresholdRsp(nResult, reqDevMap);
		}
		response.SetResponseXml(rsp);
		return 0;
	}
	
	int MsgProcess::OnSetThreshold( CMMMsg& request, CMMMsg & response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement value = info.GetSubElement("Value");
		ISFIT::CXmlElement deviceList = value.GetSubElement("DeviceList");
		std::map<CData, std::list<TThreshold> > devMap;
		CProtocolDecode::DecodeSetThreshold(deviceList, devMap);
		bool bOK = true;
		CData failedCause="NULL";
		if(devMap.size() == 0)
		{
			failedCause="请求体未识别到设备id";
			bOK = false;
		}
		else
		{
			auto pos = devMap.begin();
			while (pos != devMap.end())
			{
				std::list<TThreshold> & pointList = pos->second;
				int ret = CMMConfig::instance()->SetThresholdConf(pos->first, pointList);
				if (ret < 0)
				{
					bOK = false;
					if (ret == -2)
						failedCause = "设置值不符合范围";
					else
						failedCause = "未查找到对应设备或id";
				}
				pos++;
			}
		}
		if (bOK)
		{
			CMMConfig::instance()->UpdateCfgFile();
		}
		CData rsp = CMMProtocolEncode::BuildSetPointRsp(bOK?CMM::SUCCESS:CMM::FAILURE,failedCause,CMM::method::SET_THRESHOLD_ACK, devMap);
		response.SetResponseXml(rsp);
		//if(devMap.size() !=0)
		//	CMMAccess::instance()->NotifySendData(devMap);  //写监控数据后 上报操作设备的监控点数据
		return 0;
	}

	int MsgProcess::OnGetFtpInfo( CMMMsg& request, CMMMsg &response )
	{
		CData rsp = CMMProtocolEncode::BuildGetFtpInfoRsp();
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnSetFtpInfo( CMMMsg& request, CMMMsg& response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		CData user = info.GetSubElement("UserName").GetElementText().convertString();
		CData password = info.GetSubElement("PassWord").GetElementText().convertString();
	
		CMMParam::instance()->AddLinuxSysUser(user,password, "/");
		int ret= CMMParam::instance()->ModifyLinuxSysPasswd(user, password);
		CData rsp ;
		if(ret==0)
		{
			//FSUUTIL::SetFtpUser(user, password);
			CMMParam::instance()->UpdateParam(CMM::param::FtpUsr, user);
			CMMParam::instance()->UpdateParam(CMM::param::FtpPasswd, password);
			rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::SUCCESS, "NULL", CMM::method::SET_FTP_ACK);
		}
		else
		{
			rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::FAILURE, "NULL", CMM::method::SET_FTP_ACK);
		}
  	
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnGetLoginInfo( CMMMsg& request, CMMMsg& response )
	{
		CData rsp = CMMProtocolEncode::BuildGetLoginInfoRsp();
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnSetLoginInfo( CMMMsg& request, CMMMsg& response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		CData user = info.GetSubElement("UserName").GetElementText().convertString();
		CData password = info.GetSubElement("PassWord").GetElementText().convertString();
		CData rsp;
		if(password.length() == 0)
		{
			rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::FAILURE, "error", CMM::method::SET_LOGININFO_ACK);
		}
		else
		{
			CMMParam::instance()->UpdateParam(CMM::param::UserName, user);
			CMMParam::instance()->UpdateParam(CMM::param::Password, password);
			rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::SUCCESS, "NULL", CMM::method::SET_LOGININFO_ACK);
		}
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnGetStorageRule( CMMMsg& request, CMMMsg & response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement deviceList = info.GetSubElement("DeviceList");
		std::map<CData, std::list<TSignal> > reqDevMap;
		CProtocolDecode::DecodeGetStorageRuleList(deviceList, reqDevMap);
		
		std::map<CData, std::list<TSignal> > rspDevMap;
		CMMConfig::instance()->GetStorageRuleConf(rspDevMap);
		
		bool bOK=true;
		CData rsp;
		int nResult = CMM::SUCCESS;
		if (0 == reqDevMap.size())
		{
			if (rspDevMap.size() == 0)
			{
				nResult = CMM::NODATA;
			}
			for (auto iter = rspDevMap.begin(); iter != rspDevMap.end(); ++iter)
			{
				std::list<TSignal>& list = iter->second;
				for (auto it = list.begin(); it != list.end(); ++it)
				{
					it->result = 1;
				}
			}
			 rsp = CMMProtocolEncode::BuildGetStorageRuleRsp(nResult, rspDevMap);
		}
		else
		{
			if (rspDevMap.size() == 0)
			{
				nResult = CMM::NODATA;
			}
			else
			{
				for (auto iter = reqDevMap.begin(); iter != reqDevMap.end(); ++iter)
				{
					CData deviceId = iter->first;
					std::list<TSignal>& recvConfig = iter->second;
					auto key = rspDevMap.find(deviceId);
					if (key == rspDevMap.end())
					{
						continue;
					}
					std::list<TSignal>& config = key->second;
					for (auto it = recvConfig.begin(); it != recvConfig.end(); ++it)
					{
						for (auto its = config.begin(); its != config.end(); ++its)
						{
							if (it->ID == its->ID)
							{
								it = its;
								it->result = 1;
							}
						}
					}
				}
			}
			rsp = CMMProtocolEncode::BuildGetStorageRuleRsp(nResult, reqDevMap);
		}
		response.SetResponseXml(rsp);
		return 0;
	}
	
	int MsgProcess::OnSetStorageRule( CMMMsg& request, CMMMsg& response )
	{
	
		ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement value = info.GetSubElement("Value");
		ISFIT::CXmlElement deviceList = value.GetSubElement("DeviceList");
		std::map<CData, std::list<TSignal> > devMap;
		CProtocolDecode::DecodeSetStorageRule(deviceList, devMap);
		bool bOK = true;
		CData failedCause="NULL";
		if(devMap.size() == 0)
		{
			failedCause="请求体未识别到设备id";
			bOK = false;
		}
		else
		{
			auto pos = devMap.begin();
			while (pos != devMap.end())
			{
				std::list<TSignal> & pointList = pos->second;
				int ret = CMMConfig::instance()->SetStorageRuleConf(pos->first, pointList);
				if (ret < 0)
				{
					bOK = false;
					if (ret == -2)
						failedCause = "设置值不符合范围";
					else
						failedCause = "未查找到对应设备或id";
				}
				pos++;
			}
		}
		if (bOK)
		{
			CMMConfig::instance()->UpdateCfgFile();
		}
		CData rsp = CMMProtocolEncode::BuildSetPointRsp(bOK?CMM::SUCCESS:CMM::FAILURE,failedCause,CMM::method::SET_STORAGERULE_ACK, devMap);
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnGetFSUInfo( CMMMsg& request, CMMMsg & response )
	{
		CData rsp = CMMProtocolEncode::GetFsuInfo();
		response.SetResponseXml(rsp);
		CMMAccess::instance()->OnHeartBeat();
		return 0;
	}

	int MsgProcess::OnUpdateFsuInterval( CMMMsg& request, CMMMsg& response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		CData interval = info.GetSubElement("Interval").GetElementText().convertString();	
		CMMParam::instance()->UpdateParam (CData(CMM::param::UpdateInterval),interval);
		//CMMParam::instance()->writeJson2File();
		CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::SUCCESS,"NULL", CMM::method::UPDATE_FSUINFO_INTERVAL_ACK);
		response.SetResponseXml(rsp);
		return 0;
	}
	
	int MsgProcess::OnTimeCheck( CMMMsg& request, CMMMsg & response )
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		TTime time = { };
		CProtocolDecode::DecodeTimeCheck(info, time);
		//Poco::DateTime dateTime(time.Years, time.Month, time.Day, time.Hour, time.Minute, time.Second);
		//CData localTime(DateTimeFormatter::format(dateTime, DateTimeFormat::SORTABLE_FORMAT));
		//dateTime.makeUTC(3600*8);
		//CData utcTime(DateTimeFormatter::format(dateTime, DateTimeFormat::SORTABLE_FORMAT));


		int ret = -1;
		LogInfo("--------OnTimeCheck:"<<time.Years<<"-"<<time.Month<<"-"<<time.Day<<" "
				<<time.Hour<<":"<<time.Minute<<":"<<time.Second);
		if(time.Years<1970||time.Years>2121||time.Month<1||time.Month>12||time.Day<1||time.Day>31 ||time.Hour<0||time.Hour>23 ||time.Minute<0||time.Minute>59||time.Second<0||time.Second>59)
		{
			ret=-1;
			LogInfo("--------time error-------");
		}
		else
		{	
			//1970-01-01 12:00:01
			char str[100] = {0};
			sprintf(str, "%d-%02d-%02d %02d:%02d:%02d:",time.Years,time.Month,time.Day,time.Hour,time.Minute,time.Second);
			CData localTime(str);
			LogInfo("--------set time:"<<localTime);
			ret =APPAPI::SetTime(localTime);
		}
		
		CData rsp;
		if(ret>=0)
			rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::SUCCESS, "NULL", CMM::method::TIME_CHECK_ACK);
		else
			rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::FAILURE, "字段错误或时间错误", CMM::method::TIME_CHECK_ACK);
		
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnReboot( CMMMsg& request, CMMMsg & response )
	{
		CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::SUCCESS, "NULL", CMM::method::SET_FSUREBOOT_ACK);
		response.SetResponseXml(rsp);	
		LogNotice("===MsgProcess::OnReboot()===");
		APPAPI::RebootSys();
		return 0;
	}
 
	int MsgProcess::OnGetTime(CMMMsg& request, CMMMsg& response)
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		CData rsp = CMMProtocolEncode::BuildGetTimeRsp(CMM::SUCCESS, "NULL", CMM::method::GET_TIME_ACK);
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnSetAcceptIP(CMMMsg& request, CMMMsg& response)
	{
		ISFIT::CXmlElement info = request.GetInfoNode();
		ISFIT::CXmlElement acceptList = info.GetSubElement("AcceptList");

		int idIndex = 0;
		ISFIT::CXmlElement list = acceptList.GetSubElement("List", idIndex);
		while (list != NULL)
		{
			CData familyType = list.GetAttribute("Type");
			std::list<CData> ipList;
			int idIndex2 = 0;
			ISFIT::CXmlElement IP = list.GetSubElement("IP", idIndex2);
			while (IP != NULL)
			{
				CData ip = IP.GetElementText();
				if (ip.length() == 0) {
					continue;
				}
				ipList.push_back(ip);
				IP = list.GetSubElement("IP", idIndex2++);
			}
			list = acceptList.GetSubElement("List", idIndex++);
			CMMConfig::instance()->addAcceptIP(familyType, ipList);
		}
		CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::SUCCESS, "NULL", CMM::method::SET_ACCEPT_IP_CONF_ACK);
		response.SetResponseXml(rsp);
		return 0;
	}

	int MsgProcess::OnSetFsuReboot(CMMMsg& request, CMMMsg& response)
	{
		CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::SUCCESS, "NULL", CMM::method::SET_FSUREBOOT_ACK);
		response.SetResponseXml(rsp);
		APPAPI::RebootSys();
		return 0;
	}

	int MsgProcess::OnLogin(CMMMsg& request, CMMMsg& response)
	{
		CData rsp = CMMProtocolEncode::BuildSetLoginRsp(CMM::SUCCESS, "NULL", CMM::method::LOGIN_ACK);
		response.SetResponseXml(rsp);
		return 0;
	}
}



