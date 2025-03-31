#include "CMMMeteTranslate.h"
#include "NetComm/CSqliteObj.h"
#include "MeteInfo.h"
#include "CMMCommonStruct.h"
#include <stdio.h>

namespace CMM
{

	void CMMMeteTranslate::Init(std::map<CData, CData>& metesMap)
	{
		ISFIT::CSqliteObj sqlite;
		CData sql = "select * from cmm_mete";
		sqlite.Query(sql);
		int count = sqlite.GetRecodCount();
		ISFIT::CSqlQueryResult record;
		for(int i =0; i < count; i++)
		{
			record = sqlite.GetRecord(i);
			metesMap[record.GetValue("innerId")] = record.GetValue("cmmId");
		}
	}

	CData CMMMeteTranslate::FromInnerToCMM(int id, int &SignalNumber)
	{
		//canyon

		int cmmId = id / 1000;
		int signalNum = id % 1000;
		char cid[10] = {0};
		sprintf(cid, "%06d",cmmId);
		CData meterId(cid);
		SignalNumber = signalNum; 
		return meterId;
	}


	CData CMMMeteTranslate::FromCMMToInner( CData cmmId, int signalNumber )
	{
		CData innerId(cmmId.convertInt()*1000 + signalNumber);
		return innerId; 
	}


	int CMMMeteTranslate::ConvertToCmmMeterType(CData meterType)
	{
		if (meterType == "AI")
		{
			return CMM::AI;
		}
		if (meterType == "DI")
		{
			return CMM::DI;
		}
		if (meterType == "DO")
		{
			return CMM::DO;
		}
		if (meterType == "AO")
		{
			return CMM::AO;
		}
		return CMM::AI;
	}

	std::string CMMMeteTranslate::ConvertToMeterType(int meterType)
	{
		if (meterType == CMM::AI)
		{
			return "AI";
		}
		if (meterType == CMM::DI)
		{
			return "DI" ;
		}
		if (meterType == CMM::DO)
		{
			return "DO";
		}
		if (meterType == CMM::AO)
		{
			return "AO";
		}
		return  "AI";
	}

	int CMMMeteTranslate::FromInnerPortTypeToCMM(int type, int dataType)
	{
		if(type == SMART_DEV_METE_INFO::ALARM)
		{
			return CMM::DI;
		}
		else
		{
			if(dataType == BUSINESSLAYER_COMMON_DATA::FLOAT){
				if(type == SMART_DEV_METE_INFO::PICK){
					return CMM::AI;
				}
				else{
					return CMM::AO;
				}
			}
			else if(dataType == BUSINESSLAYER_COMMON_DATA::ENUM_TYPE){
				if(type == SMART_DEV_METE_INFO::PICK){
					return CMM::DI;
				}
				else{
					return CMM::DO;
				}
			}
			else
			{
				return -1;
			}
		}		
	}

}



