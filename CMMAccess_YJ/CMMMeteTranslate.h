#ifndef _CMMMETETRANSLATE_H
#define _CMMMETETRANSLATE_H
#include "map"
#include "Data.h"


namespace CMM
{
	class CMMMeteTranslate
	{
	public:
	
		static void Init(std::map<CData, CData>& metesMap);
		
		static CData FromCMMToInner( CData cmmId, int signalNumber );

		static int FromInnerPortTypeToCMM(int type, int dataType);

		static int ConvertToCmmMeterType(CData meterType);
		
		static std::string ConvertToMeterType(int meterType);

		static CData FromInnerToCMM(int id, int &SignalNumber);

	};
}
#endif
