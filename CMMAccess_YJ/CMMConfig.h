#ifndef _CMMCONFIG_H
#define _CMMCONFIG_H
#include "Data.h"
#include "NetComm/CXmlElement.h"
#include "CMMCommonStruct.h"
#include "CMMDeviceConfig.h"
#include "SmartLock.h"
namespace CMM{
	class CMMConfig
	{
	public:
		static CMMConfig* instance();
		int Init();
		void ReadCMMConfigData();
		CData GetDictionaryName(CData id);
		void CreateConfigFile();
		bool ReadDeviceConfig();
		CData NMAlarmID(CData signalId);
		std::map<CData, TDevConf>& GetDevices();
		bool OnUpdateCfgFileTimer();
		void UpdateCfgFile();
		void ReadDevCfgFromObj(std::list <CData>& devIdList);
		int GetDevMetes(TDevConf& cfg);
		void SaveFile();
		int GetDev(CData devId, TDevConf& cfg);

		int GetDevConf(CData devid, TDevConf& cfg);
		int SetDevCfg(std::map<CData, TDevConf>& devMap, std::list<CData>& scucessList, std::list<CData>& failList);

		void GetSemaphoreConf(std::map<CData, std::list<TSemaphore>>& reqDevMap);
		int SetSemaphoreConf(CData devid, std::list<TSemaphore>& cfg);
		void GetThresholdConf(std::map<CData, std::list<TThreshold>>& reqDevMap);
		int SetThresholdConf(CData devid, std::list<TThreshold>& cfg);
		void GetStorageRuleConf(std::map<CData, std::list<TSignal>>& reqDevMap);
		int SetStorageRuleConf(CData devid, std::list<TSignal>& cfg);

		int SetMeteValues(std::map<CData, CData>& param, TSemaphore& semaphore, int nType);
		int SetMeteStorageRule(std::map<CData, CData>& param, TSignal& Signal, int nType);
		int SetMeteThreshold(std::map<CData, CData>& param, TThreshold& theshold, int nType);
		void addAcceptIP(CData familyType, std::list<CData>& IPList);
		bool isAcceptIp(CData familyType, CData ip);
		CData CreateMeasurefile(CData& timestamp);
		bool WriteMeasurefile();
		Poco::SharedPtr<CMMDeviceConfig> GetDeviceConfig();
	private:
		static CMMConfig *_instance;
		std::map<CData, TDevConf> m_devCfg;
		std::map<CData, CData> m_dictionary; //编号 -- 名称
		std::map<CData, TDeviceInfo> m_aliasId2Info; //设备别名ID---》设备信息
		std::list <CData> m_devIdList;
		std::map <CData,int> m_dev2MeterList; //每个dev下 量的个数
		Poco::SharedPtr<CMMDeviceConfig> m_pDeviceConfig;
	public:
		ISFIT::CSmartMutex m_devCfgMutex;
		ISFIT::CXmlDoc m_doc;
		std::map<CData, std::list<CData>> m_familyIPList;
		CData m_DevCfgFileName;
		bool m_bUpdate;
		bool m_bUpdateBak;
	};
}
#endif
