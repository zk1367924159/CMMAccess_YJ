//canyon 2019 0902

#include <fstream>  
#include "CLog.h"
#include "CMMParam.h"
#include "CMMCommonStruct.h"
#include "CMMConfig.h"
#include "SysCommon.h"
#include "CMMAccess.h"
#include "Poco/SharedPtr.h"
#include "Poco/DateTimeFormatter.h"  
#include "Poco/DateTime.h" 
#include "../../ExtAppIpc/ExtAppIpcApi.h"
//#include "ExtSoApi.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#define  CMM_PARAM_CONFIG  "/appdata/config/MobileBConfig.json"
namespace CMM{
	CMMParam* CMMParam::_instance = NULL;

	CMMParam* CMMParam::instance()
	{
		if (_instance == NULL)
		{
			_instance = new CMMParam();
		}
		return _instance;
	}

	int CMMParam::AddLinuxSysUser(CData user, CData passwd, CData dir)
	{	
		//system("mount -o remount,rw /");
		//Poco::Thread::sleep(500);
		if (user.empty()) return -1;
		
		FILE   *shellFile; 
		int ret = 0;
		CData cmd = "adduser " + user; 
		if (dir.size()>0)
		{
			cmd += " -h " + dir;
		}
		if (passwd.empty())
		{
			cmd += " -D";
		}
		
		if ((shellFile = popen(cmd.c_str(), "w") ) == nullptr) 
		{ 
			perror("popen");
			LogError("popen error:"<<strerror(errno));
			ret = -1; 
		} 
		else
		{
			LogInfo("Shell cmd: "<<cmd);
	
			if (passwd.size() > 0)
			{
				Poco::Thread::sleep(500);
				passwd += "\r";
				LogInfo("=====>shell write the passwd:"<<passwd);
				fwrite(passwd.c_str(), 1, passwd.size(), shellFile);
				
				Poco::Thread::sleep(500);
				LogInfo("=====>shell write the passwd again");
				fwrite(passwd.c_str(), 1, passwd.size(), shellFile);
			}
			
			if ((ret = pclose(shellFile)) == -1) 
			{ 
				LogError("close popen error, cmd:"<<cmd<<" ret:"<<ret);
				ret = -2;
			} 
		}
	
		//system("mount -o remount,ro /");
		//Poco::Thread::sleep(500);
		
		return ret;
	}
	
	int CMMParam::DelLinuxSysUser(CData user)
	{	
		if (user.empty()) return -1;
		//system("mount -o remount,rw /");
		//Poco::Thread::sleep(500);
		
		CData cmd = "deluser " + user; 
		ISFIT::Shell(cmd);	
	
		//system("mount -o remount,ro /");
		//Poco::Thread::sleep(500);
		
		return 0;
	}
	
	
	
	int CMMParam::ModifyLinuxSysPasswd(CData user, CData passwd)
	{	
		//system("mount -o remount,rw /");
		//Poco::Thread::sleep(500);
		if (user.empty() || passwd.empty()) return -1;

		
		FILE   *shellFile; 
		int ret = 0;
	
		CData cmd = "passwd " + user; 
		
		if ((shellFile = popen(cmd.c_str(), "w") ) == nullptr) 
		{ 
			perror("popen");
			LogError("popen error:"<<strerror(errno));
			ret = -1; 
		} 
		else
		{
			LogInfo("Shell cmd: "<<cmd);
			
			Poco::Thread::sleep(500);
			passwd += "\r";
			LogInfo("=====>modify passwd:"<<passwd);
			fwrite(passwd.c_str(), 1, passwd.size(), shellFile);
			
			Poco::Thread::sleep(500);
			LogInfo("=====>modify passwd again");
			fwrite(passwd.c_str(), 1, passwd.size(), shellFile);
			
			if ((ret = pclose(shellFile)) == -1) 
			{ 
				LogError("close popen error, cmd:"<<cmd<<" ret:"<<ret);
				ret = -2;
			} 
		}
	
		//system("mount -o remount,ro /");
		//Poco::Thread::sleep(500);
		
		return ret;
	}

	void CMMParam::initParam()
	{
		/*m_AlarmSendDB =  GetParam(CMM::param::AlarmSendDB, "");
		m_LoggerChannel =  GetParam(CMM::param::LoggerChannel, "");
		m_FlowControl =  GetParam(CMM::param::FlowControl, "");*/
		m_FsuDeviceId =  GetParam(CMM::param::FsuDeviceId, "");

		//m_CsvEncoding = GetParam(CMM::param::CsvEncoding, "");
		m_CsvExpire = GetParam(CMM::param::CsvExpire, "");
		m_CsvMeasurementTime = GetParam(CMM::param::CsvMeasurementTime, "");
		m_ScDoorIp = GetParam(CMM::param::SCDoorIp, "");
		m_ScDoorPort = GetParam(CMM::param::SCDoorPort, "");
		m_ScDoorTransPort = GetParam(CMM::param::SCDoorTransPort, "");
		m_ScProtocol = GetParam(CMM::param::SCProtocol, "");
		m_AuthEnable = GetParam(CMM::param::AuthEnable, "");
		m_EnginState = GetParam(CMM::param::EnginState, "");
		m_UpdateEnable = GetParam(CMM::param::UpdateEnable, "");
		m_SoapEnable = GetParam(CMM::param::SoapEnable, "");
		m_RebootEnable = GetParam(CMM::param::RebootEnable, "");
		m_RebootPeriod = GetParam(CMM::param::RebootPeriod, "");
		m_RebootTime = GetParam(CMM::param::RebootTime, "");
		m_FsuId = GetParam(CMM::param::FsuId, "");
		m_FsuEndPoint = GetParam(CMM::param::FsuEndPoint, "");
		m_FtpUsr = GetParam(CMM::param::FtpUsr, "");
		m_FtpPasswd = GetParam(CMM::param::FtpPasswd, "");
		m_FtpType = GetParam(CMM::param::FtpType, "");
		m_LogFileSize = GetParam(CMM::param::LogFileSize, "");
		m_LogLevel = GetParam(CMM::param::LogLevel, "");
		m_LoggerCount = GetParam(CMM::param::LoggerCount, "");

		m_LoginPeriod = GetParam(CMM::param::LoginPeriod, "");
		m_Algorithm = GetParam(CMM::param::Algorithm, "");
		m_Password = GetParam(CMM::param::Password, "");
		m_UserName = GetParam(CMM::param::UserName, "");
		m_ScEndPoint = GetParam(CMM::param::SCEndPoint, "");
		m_SendPeriod = GetParam(CMM::param::SendPeriod, "");
		m_UpdateInterval = GetParam(CMM::param::UpdateInterval, "");

		m_WebDeviceConfig = GetParam(CMM::param::WebDeviceConfig, "");
		//m_WebQueueDepth = GetParam(CMM::param::WebQueueDepth, "");
		//m_WebInterval = GetParam(CMM::param::WebInterval, "");
		m_WebHost = GetParam(CMM::param::WebHost, "");
		//m_WebHtdocs = GetParam(CMM::param::WebHtdocs, "");
		//m_WebMimes = GetParam(CMM::param::WebMimes, "");
		m_WebPort = GetParam(CMM::param::WebPort, "");
		//m_WebStorageDevice = GetParam(CMM::param::WebStorageDevice, "");
		m_WebVersion = GetParam(CMM::param::WebVersion, "");

		m_SiteID = GetParam(CMM::param::SiteID, "");
		m_SiteName = GetParam(CMM::param::SiteName, "");
		m_RoomID = GetParam(CMM::param::RoomID, "");
		m_RoomName = GetParam(CMM::param::RoomName, "");
		m_UartName = GetParam(CMM::param::UartName, "");
		m_BaudRate = GetParam(CMM::param::BaudRate, "");
		m_DataBit = GetParam(CMM::param::DataBit, "");
		m_Parity = GetParam(CMM::param::Parity, "");
		m_StopBit = GetParam(CMM::param::StopBit, "");
		m_SlaveID = GetParam(CMM::param::SlaveID, "");

		m_ScUdpPoint = m_ScProtocol + "://" + m_ScDoorIp + ":" + m_ScDoorPort;// 透传
	}

	void CMMParam::initJsonFile()
	{
		/*m_json[CMM::param::AlarmSendDB] = m_AlarmSendDB.c_str();
		m_json[CMM::param::LoggerChannel] = m_LoggerChannel.c_str();
		m_json[CMM::param::FlowControl] = m_FlowControl.c_str();*/
		m_json[CMM::param::FsuDeviceId] = m_FsuDeviceId.c_str();

        //m_json[CMM::param::CsvEncoding] = m_CsvEncoding.c_str();
        m_json[CMM::param::CsvExpire] = m_CsvExpire.c_str();
        m_json[CMM::param::CsvMeasurementTime] = m_CsvMeasurementTime.c_str();
        m_json[CMM::param::SCDoorIp] = m_ScDoorIp.c_str();
        m_json[CMM::param::SCDoorPort] = m_ScDoorPort.c_str();
        m_json[CMM::param::SCDoorTransPort] = m_ScDoorTransPort.c_str();
        m_json[CMM::param::SCProtocol] = m_ScProtocol.c_str();
        m_json[CMM::param::AuthEnable] = m_AuthEnable.c_str();
        m_json[CMM::param::EnginState] = m_EnginState.c_str();
        m_json[CMM::param::UpdateEnable] = m_UpdateEnable.c_str();
        m_json[CMM::param::SoapEnable] = m_SoapEnable.c_str();
        m_json[CMM::param::RebootEnable] = m_RebootEnable.c_str();
        
        m_json[CMM::param::RebootPeriod] = m_RebootPeriod.c_str();
        m_json[CMM::param::RebootTime] = m_RebootTime.c_str();
        m_json[CMM::param::FsuId] = m_FsuId.c_str();
        m_json[CMM::param::FsuEndPoint] = m_FsuEndPoint.c_str();
        m_json[CMM::param::FtpUsr] = m_FtpUsr.c_str();
        m_json[CMM::param::FtpPasswd] = m_FtpPasswd.c_str();
        m_json[CMM::param::FtpType] = m_FtpType.c_str();
        m_json[CMM::param::LogFileSize] = m_LogFileSize.c_str();
        m_json[CMM::param::LogLevel] = m_LogLevel.c_str();
		//m_json[CMM::param::LoggerCount] = m_LoggerCount.c_str();

        m_json[CMM::param::LoginPeriod] = m_LoginPeriod.c_str();
        m_json[CMM::param::Algorithm] = m_Algorithm.c_str();
        m_json[CMM::param::Password] = m_Password.c_str();
        m_json[CMM::param::UserName] = m_UserName.c_str();
        m_json[CMM::param::SCEndPoint] = m_ScEndPoint.c_str();
        m_json[CMM::param::SendPeriod] = m_SendPeriod.c_str();
        m_json[CMM::param::UpdateInterval] = m_UpdateInterval.c_str();
        
        m_json[CMM::param::WebDeviceConfig] = m_WebDeviceConfig.c_str();
        //m_json[CMM::param::WebQueueDepth] = m_WebQueueDepth.c_str();
        //m_json[CMM::param::WebInterval] = m_WebInterval.c_str();
        m_json[CMM::param::WebHost] = m_WebHost.c_str();
        //m_json[CMM::param::WebHtdocs] = m_WebHtdocs.c_str();
        //m_json[CMM::param::WebMimes] = m_WebMimes.c_str();
        m_json[CMM::param::WebPort] = m_WebPort.c_str();
       // m_json[CMM::param::WebStorageDevice] = m_WebStorageDevice.c_str();
        m_json[CMM::param::WebVersion] = m_WebVersion.c_str();
        
        m_json[CMM::param::SiteID] = m_SiteID.c_str();
        m_json[CMM::param::SiteName] = m_SiteName.c_str();
        m_json[CMM::param::RoomID] = m_RoomID.c_str();
        m_json[CMM::param::RoomName] = m_RoomName.c_str();
        m_json[CMM::param::UartName] = m_UartName.c_str();
        m_json[CMM::param::BaudRate] = m_BaudRate.c_str();
        m_json[CMM::param::DataBit] = m_DataBit.c_str();
        m_json[CMM::param::Parity] = m_Parity.c_str();
        m_json[CMM::param::StopBit] = m_StopBit.c_str();
        m_json[CMM::param::SlaveID] = m_SlaveID.c_str();

	}
	bool CMMParam::writeJson2File()
	{
		LogInfo("writeJson2File");
		const std::string filePath = CMM_PARAM_CONFIG; // 确保 CMM_PARAM_CONFIG 是一个有效的文件路径字符串

		// 使用 ofstream 打开文件，如果文件不存在则创建，如果已存在则覆盖
		std::ofstream ofs(filePath, std::ios::out | std::ios::trunc); // std::ios::trunc 表示截断文件，即覆盖内容
		if (!ofs)
		{
			// 如果文件无法打开（可能是由于权限问题或其他原因），则记录错误并返回 false
			LogError("Could not open file for writing: " << filePath);
			return false;
		}

		try
		{
			// 将 JSON 数据写入文件
			ofs << m_json.dump(2);
			ofs.flush(); // 确保数据被写入文件

			// 如果需要，可以在这里记录成功写入的信息
			// LogInfo("Successfully wrote JSON data to file: " << filePath);
		}
		catch (const Poco::Exception& ex)
		{
			// 捕获 Poco 异常并记录错误信息
			LogError("Error writing file: " << ex.displayText());
			return false;
		}
		catch (const std::exception& ex)
		{
			// 捕获标准库异常并记录错误信息
			LogError("Standard exception while writing file: " << ex.what());
			return false;
		}

		// 如果代码执行到这里，说明文件已成功写入
		return true;
	}

	bool CMMParam::Init()
	{
		initParam();
		initJsonFile();
		return true;
		//return writeJson2File();
	}

	std::string CMMParam::LoadParams()
	{
		return m_json.dump(2);
	}

	void CMMParam::SavaParams(std::string content)
	{
		try
		{
			json tempJson = json::parse(content);

			for (auto it = tempJson.begin(); it != tempJson.end(); ++it)
			{
				std::string key = it.key();
				json value = it.value();
				if (value.is_null())
				{
					LogInfo("this key is null. " << key);
					UpdateParam(key.c_str(),"");
					continue;
				}
				if (!value.is_string())
				{
					LogInfo("this key is: " << key);
					continue;
				}
				std::string val = value.get<std::string>();
				try
				{
					if (m_json[key].get<std::string>() != val)
					{
						LogInfo("key : " << key << "val: " << val);
						UpdateParam(key.c_str(), val.c_str());
					}
				}
				catch (const std::exception& e)
				{
					LogError("key: " << key << "exception: " << e.what());
					continue;
				}
			}
			//writeJson2File();
		}
		catch (const std::exception& e)
		{
			LogError("exception: " << e.what());
		}

	}

	int CMMParam::UpdateParam(CData key, CData val)
	{
		if (key == CMM::param::CsvEncoding)
		{
			m_CsvEncoding = val.c_str();
			m_json[CMM::param::CsvEncoding] = m_CsvEncoding.c_str();
		}
		else if (key == CMM::param::CsvExpire)
		{
			m_CsvExpire = val.c_str();
			m_json[CMM::param::CsvExpire] = m_CsvExpire.c_str();
		}
		else if (key == CMM::param::CsvMeasurementTime)
		{
			m_CsvMeasurementTime = val.c_str();
			m_json[CMM::param::CsvMeasurementTime] = m_CsvMeasurementTime.c_str();
		}
		else if (key == CMM::param::SCDoorIp)
		{
			m_ScDoorIp = val.c_str();
			m_json[CMM::param::SCDoorIp] = m_ScDoorIp.c_str();
			m_ScUdpPoint = m_ScProtocol + "://" + m_ScDoorIp + ":" + m_ScDoorPort;
		}
		else if (key == CMM::param::SCDoorPort)
		{
			m_ScDoorPort = val.c_str();
			m_json[CMM::param::SCDoorPort] = m_ScDoorPort.c_str();
			m_ScUdpPoint = m_ScProtocol + "://" + m_ScDoorIp + ":" + m_ScDoorPort;
		}
		else if (key == CMM::param::SCDoorTransPort)
		{
			m_ScDoorTransPort = val.c_str();
			m_json[CMM::param::SCDoorTransPort] = m_ScDoorTransPort.c_str();
			CMMAccess::instance()->setDoorServeParam(key,m_ScProtocol,val);
		}
		else if (key == CMM::param::SCProtocol)
		{
			m_ScProtocol = val.c_str();
			m_json[CMM::param::SCProtocol] = m_ScProtocol.c_str();
			m_ScUdpPoint = m_ScProtocol + "://" + m_ScDoorIp + ":" + m_ScDoorPort;
			CMMAccess::instance()->setDoorServeParam(key,val,m_ScDoorTransPort);
		}
		else if (key == CMM::param::AuthEnable)
		{
			m_AuthEnable = val.c_str();
			m_json[CMM::param::AuthEnable] = m_AuthEnable.c_str();
		}
		else if (key == CMM::param::EnginState)
		{
			m_EnginState = val.c_str();
			m_json[CMM::param::EnginState] = m_EnginState.c_str();
		}
		else if (key == CMM::param::UpdateEnable)
		{
			m_UpdateEnable = val.c_str();
			m_json[CMM::param::UpdateEnable] = m_UpdateEnable.c_str();
		}
		else if (key == CMM::param::SoapEnable)
		{
			m_SoapEnable = val.c_str();
			m_json[CMM::param::SoapEnable] = m_SoapEnable.c_str();
		}
		else if (key == CMM::param::RebootEnable)
		{
			m_RebootEnable = val.c_str();
			m_json[CMM::param::RebootEnable] = m_RebootEnable.c_str();
		}
		else if (key == CMM::param::RebootPeriod)
		{
			m_RebootPeriod = val.c_str();
			m_json[CMM::param::RebootPeriod] = m_RebootPeriod.c_str();
		}
		else if (key == CMM::param::RebootTime)
		{
			m_RebootTime = val.c_str();
			m_json[CMM::param::RebootTime] = m_RebootTime.c_str();
		}
		else if (key == CMM::param::FsuId)
		{
			m_FsuId = val.c_str();
			m_json[CMM::param::FsuId] = m_FsuId.c_str();
		}
		else if (key == CMM::param::FsuEndPoint)
		{
			m_FsuEndPoint = val.c_str();
			m_json[CMM::param::FsuEndPoint] = m_FsuEndPoint.c_str();
			CMMAccess::instance()->setHttpParam(key,val);
		}
		else if (key == CMM::param::FtpUsr)
		{
			m_json[CMM::param::FtpUsr] = m_FtpUsr.c_str();
			DelLinuxSysUser(m_FtpUsr);
			m_FtpUsr = val.c_str();
			if (m_FtpType == "ftp")
				AddLinuxSysUser(m_FtpUsr, m_FtpPasswd, "/home/ftp");
			else
				AddLinuxSysUser(m_FtpUsr, m_FtpPasswd, "/userdata/ftp");
		}
		else if (key == CMM::param::FtpPasswd)
		{
			m_FtpPasswd = val.c_str();
			m_json[CMM::param::FtpPasswd] = m_FtpPasswd.c_str();
			ModifyLinuxSysPasswd(m_FtpUsr,m_FtpPasswd);
		}
		else if (key == CMM::param::FtpType)
		{
			m_FtpType = val.c_str();
			m_json[CMM::param::FtpType] = m_FtpType.c_str();
		}
		else if (key == CMM::param::LogFileSize)
		{
			m_LogFileSize = val.c_str();
			m_json[CMM::param::LogFileSize] = m_LogFileSize.c_str();
		}
		else if (key == CMM::param::LogLevel)
		{
			m_LogLevel = val.c_str();
			m_json[CMM::param::LogLevel] = m_LogLevel.c_str();
			APPAPI::SetLogLevel(GetLogLevel(val));
		}
		else if (key == CMM::param::LoginPeriod)
		{
			m_LoginPeriod = val.c_str();
			m_json[CMM::param::LoginPeriod] = m_LoginPeriod.c_str();
		}
		else if (key == CMM::param::Algorithm)
		{
			m_Algorithm = val.c_str();
			m_json[CMM::param::Algorithm] = m_Algorithm.c_str();
		}
		else if (key == CMM::param::Password)
		{
			m_Password = val.c_str();
			m_json[CMM::param::Password] = m_Password.c_str();
		}
		else if (key == CMM::param::UserName)
		{
			m_UserName = val.c_str();
			m_json[CMM::param::UserName] = m_UserName.c_str();
		}
		else if (key == CMM::param::SCEndPoint)
		{
			m_ScEndPoint = val.c_str();
			m_json[CMM::param::SCEndPoint] = m_ScEndPoint.c_str();
		}
		else if (key == CMM::param::SendPeriod)
		{
			m_SendPeriod = val.c_str();
			m_json[CMM::param::SendPeriod] = m_SendPeriod.c_str();
		}
		else if (key == CMM::param::UpdateInterval)
		{
			m_UpdateInterval = val.c_str();
			m_json[CMM::param::UpdateInterval] = m_UpdateInterval.c_str();
		}
		/*else if (key == CMM::param::WebDeviceConfig)
		{
			m_WebDeviceConfig = val.c_str();
			m_json[CMM::param::WebDeviceConfig] = m_WebDeviceConfig.c_str();
		}
		else if (key == CMM::param::WebQueueDepth)
		{
			m_WebQueueDepth = val.c_str();
			m_json[CMM::param::WebQueueDepth] = m_WebQueueDepth.c_str();
		}
		else if (key == CMM::param::WebInterval)
		{
			m_WebInterval = val.c_str();
			m_json[CMM::param::WebInterval] = m_WebInterval.c_str();
		}
		else if (key == CMM::param::WebHost)
		{
			m_WebHost = val.c_str();
			m_json[CMM::param::WebHost] = m_WebHost.c_str();
		}
		else if (key == CMM::param::WebHtdocs)
		{
			m_WebHtdocs = val.c_str();
			m_json[CMM::param::WebHtdocs] = m_WebHtdocs.c_str();
		}
		else if (key == CMM::param::WebMimes)
		{
			m_WebMimes = val.c_str();
			m_json[CMM::param::WebMimes] = m_WebMimes.c_str();
		}*/
		else if (key == CMM::param::WebPort)
		{
			m_WebPort = val.c_str();
			m_json[CMM::param::WebPort] = m_WebPort.c_str();
			CMMAccess::instance()->setWebParam(key,val);
		}
		/*else if (key == CMM::param::WebStorageDevice)
		{
			m_WebStorageDevice = val.c_str();
			m_json[CMM::param::WebStorageDevice] = m_WebStorageDevice.c_str();
		}
		else if (key == CMM::param::WebVersion)
		{
			m_WebVersion = val.c_str();
			m_json[CMM::param::WebVersion] = m_WebVersion.c_str();
		}*/
		else if (key == CMM::param::SiteID)
		{
			m_SiteID = val.c_str();
			m_json[CMM::param::SiteID] = m_SiteID.c_str();
		}
		else if (key == CMM::param::SiteName)
		{
			m_SiteName = val.c_str();
			m_json[CMM::param::SiteName] = m_SiteName.c_str();
		}
		else if (key == CMM::param::RoomID)
		{
			m_RoomID = val.c_str();
			m_json[CMM::param::RoomID] = m_RoomID.c_str();
		}
		else if (key == CMM::param::RoomName)
		{
			m_RoomName = val.c_str();
			m_json[CMM::param::RoomName] = m_RoomName.c_str();
		}
		else if (key == CMM::param::UartName)
		{
			m_UartName = val.c_str();
			m_json[CMM::param::UartName] = m_UartName.c_str();
			CMMAccess::instance()->setUartParam(key,val);
		}
		else if (key == CMM::param::BaudRate)
		{
			m_BaudRate = val.c_str();
			m_json[CMM::param::BaudRate] = m_BaudRate.c_str();
			CMMAccess::instance()->setUartParam(key,val);
		}
		else if (key == CMM::param::DataBit)
		{
			m_DataBit = val.c_str();
			m_json[CMM::param::DataBit] = m_DataBit.c_str();
			CMMAccess::instance()->setUartParam(key,val);
		}
		else if (key == CMM::param::Parity)
		{
			m_Parity = val.c_str();
			m_json[CMM::param::Parity] = m_Parity.c_str();
			CMMAccess::instance()->setUartParam(key,val);
		}
		else if (key == CMM::param::StopBit)
		{
			m_StopBit = val.c_str();
			m_json[CMM::param::StopBit] = m_StopBit.c_str();
			CMMAccess::instance()->setUartParam(key,val);
		}
		else if (key == CMM::param::SlaveID)
		{
			m_SlaveID = val.c_str();
			m_json[CMM::param::SlaveID] = m_SlaveID.c_str();
			CMMAccess::instance()->setUartParam(key,val);
		}
		else
		{
			return -1;
		}
		SetParam(key,val);
		return 0;
	}

	CData CMMParam::GetParam(CData key, CData defVal)
	{
		CData val = APPAPI::GetExtAppParam(key);
		if (val.size()>0) return val;
		return defVal;
	}

	int CMMParam::SetParam(CData key, CData val)
	{
		LogInfo("SetParam: "  << key.c_str() << " value:" << val.c_str());
		if (0 == APPAPI::SaveExtAppParam(key, val))
		{
			m_json[key.c_str()] = val.c_str();
			return 0;
		}
		return -1;
	}

	CData CMMParam::GetLogLevel(CData value)
	{
		int level = value.convertInt();
		switch (level)
		{
		case 1:
			return "fatal";
		case 2:
			return "critical";
		case 3:
			return "warning";
		case 4:
			return "notice";
		case 5:
			return "information";
		case 6:
			return "debug";
		case 7:
			return "trace";
		default:
			return "information";
		}
	}
}