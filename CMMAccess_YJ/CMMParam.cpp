//canyon 2019 0902

#include <fstream>  
#include "CLog.h"
#include "CMMParam.h"
#include "CMMCommonStruct.h"
#include "CMMConfig.h"
#include "Poco/SharedPtr.h"
#include "Poco/DateTimeFormatter.h"  
#include "Poco/DateTime.h" 
#include "../../ExtAppIpc/ExtAppIpcApi.h"

#define  CMM_PARAM_CONFIG  "appdata/config/MobileBConfig.json"
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

	void CMMParam::initParam()
	{
		m_CsvEncoding = GetParam(CMM::param::CsvEncoding, "");
		m_CsvExpire = GetParam(CMM::param::CsvExpire, "");
		m_CsvMeasurementTime = GetParam(CMM::param::CsvMeasurementTime, "");
		m_ScIp = GetParam(CMM::param::SCIp, "");
		m_ScPort = GetParam(CMM::param::SCPort, "");
		m_ScUdpPort = GetParam(CMM::param::SCUdpPort, "");
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
		m_LoginPeriod = GetParam(CMM::param::LoginPeriod, "");
		m_Algorithm = GetParam(CMM::param::Algorithm, "");
		m_Password = GetParam(CMM::param::Password, "");
		m_UserName = GetParam(CMM::param::UserName, "");
		m_ScEndPoint = GetParam(CMM::param::SCEndPoint, "");
		m_SendPeriod = GetParam(CMM::param::SendPeriod, "");
		m_UpdateInterval = GetParam(CMM::param::UpdateInterval, "");

		//m_webDeviceDB = GetParam(CMM::param::WebDevicedDB, "");
		//m_webQueueDepth = GetParam(CMM::param::WebQueueDepth, "");
		m_WebInterval = GetParam(CMM::param::WebInterval, "");
		m_WebHost = GetParam(CMM::param::WebHost, "");
		m_WebHtdocs = GetParam(CMM::param::WebHtdocs, "");
		m_WebMimes = GetParam(CMM::param::WebMimes, "");
		m_WebPort = GetParam(CMM::param::WebPort, "");
		m_WebStorageDevice = GetParam(CMM::param::WebStorageDevice, "");
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

		m_scUdpPoint = m_ScProtocol+ "://"+ m_ScIp + ":"+ m_ScPort + "/v1/services/newLSCService";//LSCService
	}

	void CMMParam::initJsonFile()
	{
        m_json[CMM::param::CsvEncoding] = m_CsvEncoding.c_str();
        m_json[CMM::param::CsvExpire] = m_CsvExpire.c_str();
        m_json[CMM::param::CsvMeasurementTime] = m_CsvMeasurementTime.c_str();
        m_json[CMM::param::SCIp] = m_ScIp.c_str();
        m_json[CMM::param::SCPort] = m_ScPort.c_str();
        m_json[CMM::param::SCUdpPort] = m_ScUdpPort.c_str();
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
        m_json[CMM::param::LoginPeriod] = m_LoginPeriod.c_str();
        m_json[CMM::param::Algorithm] = m_Algorithm.c_str();
        m_json[CMM::param::Password] = m_Password.c_str();
        m_json[CMM::param::UserName] = m_UserName.c_str();
        m_json[CMM::param::SCEndPoint] = m_ScEndPoint.c_str();
        m_json[CMM::param::SendPeriod] = m_SendPeriod.c_str();
        m_json[CMM::param::UpdateInterval] = m_UpdateInterval.c_str();
        
        m_json[CMM::param::WebDevicedDB] = m_WebDevicedDB.c_str();
        m_json[CMM::param::WebQueueDepth] = m_WebQueueDepth.c_str();
        m_json[CMM::param::WebInterval] = m_WebInterval.c_str();
        m_json[CMM::param::WebHost] = m_WebHost.c_str();
        m_json[CMM::param::WebHtdocs] = m_WebHtdocs.c_str();
        m_json[CMM::param::WebMimes] = m_WebMimes.c_str();
        m_json[CMM::param::WebPort] = m_WebPort.c_str();
        m_json[CMM::param::WebStorageDevice] = m_WebStorageDevice.c_str();
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
		std::ofstream file(CMM_PARAM_CONFIG);
		if (!file.is_open()) 
		{
			LogError("Could not open file: " << CMM_PARAM_CONFIG);
			return false;
		}
		file << m_json.dump(4); // Write JSON with indent of 4 spaces
		return true;
	}

	bool CMMParam::Init()
	{
		initParam();
		initJsonFile();
		return writeJson2File();
	}

	json& CMMParam::GetJsonObj()
	{
		return m_json;
	}

	void CMMParam::SetJsonObj(json& jsonObj)
	{
		m_json = jsonObj;
		writeJson2File();
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
		else if (key == CMM::param::SCIp)
		{
			m_ScIp = val.c_str();
			m_json[CMM::param::SCIp] = m_ScIp.c_str();
			m_scUdpPoint = m_ScProtocol+ "://"+ m_ScIp + ":"+ m_ScPort + "/v1/services/newLSCService";//LSCService

		}
		else if (key == CMM::param::SCPort)
		{
			m_ScPort = val.c_str();
			m_json[CMM::param::SCPort] = m_ScPort.c_str();
			m_scUdpPoint = m_ScProtocol+ "://"+ m_ScIp + ":"+ m_ScPort + "/v1/services/newLSCService";//LSCService
		}
		else if (key == CMM::param::SCUdpPort)
		{
			m_ScUdpPort = val.c_str();
			m_json[CMM::param::SCUdpPort] = m_ScUdpPort.c_str();
		}
		else if (key == CMM::param::SCProtocol)
		{
			m_ScProtocol = val.c_str();
			m_json[CMM::param::SCProtocol] = m_ScProtocol.c_str();
			m_scUdpPoint = m_ScProtocol+ "://"+ m_ScIp + ":"+ m_ScPort + "/v1/services/newLSCService";//LSCService

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
		}
		else if (key == CMM::param::FtpUsr)
		{
			m_FtpUsr = val.c_str();
			m_json[CMM::param::FtpUsr] = m_FtpUsr.c_str();
		}
		else if (key == CMM::param::FtpPasswd)
		{
			m_FtpPasswd = val.c_str();
			m_json[CMM::param::FtpPasswd] = m_FtpPasswd.c_str();
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

		else if (key == CMM::param::WebDevicedDB)
		{
			m_WebDevicedDB = val.c_str();
			m_json[CMM::param::WebDevicedDB] = m_WebDevicedDB.c_str();
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
		}
		else if (key == CMM::param::WebPort)
		{
			m_WebPort = val.c_str();
			m_json[CMM::param::WebPort] = m_WebPort.c_str();
		}
		else if (key == CMM::param::WebStorageDevice)
		{
			m_WebStorageDevice = val.c_str();
			m_json[CMM::param::WebStorageDevice] = m_WebStorageDevice.c_str();
		}
		else if (key == CMM::param::WebVersion)
		{
			m_WebVersion = val.c_str();
			m_json[CMM::param::WebVersion] = m_WebVersion.c_str();
		}

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
		}
		else if (key == CMM::param::BaudRate)
		{
			m_BaudRate = val.c_str();
			m_json[CMM::param::BaudRate] = m_BaudRate.c_str();
		}
		else if (key == CMM::param::DataBit)
		{
			m_DataBit = val.c_str();
			m_json[CMM::param::DataBit] = m_DataBit.c_str();
		}
		else if (key == CMM::param::Parity)
		{
			m_Parity = val.c_str();
			m_json[CMM::param::Parity] = m_Parity.c_str();
		}
		else if (key == CMM::param::StopBit)
		{
			m_StopBit = val.c_str();
			m_json[CMM::param::StopBit] = m_StopBit.c_str();
		}
		else if (key == CMM::param::SlaveID)
		{
			m_SlaveID = val.c_str();
			m_json[CMM::param::SlaveID] = m_SlaveID.c_str();
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
		if (0 == APPAPI::SaveExtAppParam(key, val))
		{
			m_json[key.c_str()] = val.c_str();
			return 0;
		}
		return -1;
	}
}