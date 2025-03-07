#include "CMMSoapXmlEncode.h"
#include <regex>
namespace CMM
{

	CData replaceAll(const CData& str, const CData from, const CData to)
	{
		CData result;
		size_t pos = 0;
		while ((pos = str.find(from, pos)) != CDATA_NPOS)        
		{
			result += str.substr(0, pos) + to;
			pos += from.length();
		}
		return result + str.substr(pos);
	}

	std::string replace_all(std::string str, const std::string& from, const std::string& to) 
	{
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) 
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
		return str;
	}

	CData CMMSoapXmlEncode::setSoapSerialization(std::string xmlData,int nType)
	{
		CData serialXml;
		if(nType == 0)
			serialXml = SERVICE_SOAP_RECV_XML_HEAD;
		else
			serialXml = CLIENT_SOAP_SEND_XML_HEAD;
		// 使用正则表达式一次性替换 <
		xmlData = replace_all(xmlData,"<","&lt;");
		xmlData = replace_all(xmlData, ">", "&gt;");
		serialXml += CData(xmlData);
		if (nType == 0)
			serialXml += CData(SERVICE_SOAP_RECV_XML_END);
		else
			serialXml += CData(CLIENT_SOAP_SEND_XML_END);
		LogInfo("serialXml2 :" << serialXml.c_str());
		return serialXml;
	}

	CData CMMSoapXmlEncode::soapClientResponseDeserialization(char* SoapxmlData)
	{
		CData xmlData;
		TiXmlDocument doc;
		const char* errorMsg = doc.Parse(SoapxmlData);
		if (errorMsg)
		{
			LogError("SoapxmlData Parse failed: " << errorMsg);
			return "";
		}
		TiXmlElement* root = doc.RootElement();
		if (!root)
		{
			LogError("Failed to parse XML, no root element found." << SoapxmlData);
			return "";
		}
		// 获取Body元素
		TiXmlElement* bodyElement = root->FirstChildElement("soap:Body");
		if (!bodyElement)
		{
			bodyElement = root->FirstChildElement("SOAP-ENV:Body");
		}
		if (bodyElement)
		{
			// 获取invokeResponse元素
			TiXmlElement* invokeResponseElement = bodyElement->FirstChildElement("ns1:invokeResponse");
			if (!invokeResponseElement)
			{
				invokeResponseElement = bodyElement->FirstChildElement("ns:invokeResponse");	
			}
			if (invokeResponseElement)
			{
				// 现在您可以访问或处理invokeResponse元素及其内容了
				// 查找invokeReturn元素  
				TiXmlElement* invokeReturnElement = invokeResponseElement->FirstChildElement("invokeReturn");
				if (!invokeReturnElement)
				{
					invokeReturnElement = invokeResponseElement->FirstChildElement("xmlData");
				}
				if (!invokeReturnElement)
				{
					LogError("invokeReturn element not found." << SoapxmlData);
					return "";
				}
				// 获取invokeReturn元素的文本内容  
				std::string pXmlData = invokeReturnElement->GetText();
				if (!pXmlData.empty())
				{
					
					pXmlData = replace_all(pXmlData, "&lt;","<");
					pXmlData = replace_all(pXmlData, "&gt;",">");
					LogInfo("xmlData : " << pXmlData);
					xmlData = pXmlData;
				}
				else {
					LogError("No text content found in invokeReturn element." << SoapxmlData);
				}
			}
			else 
			{
				LogError("invokeResponse element not found in SOAP Body." << SoapxmlData);
			}
		}
		else
		{
			LogError("SOAP Body element not found." << SoapxmlData);
		}
		return xmlData;
	}

	CData CMMSoapXmlEncode::soapServerResquestDeserialization(char* SoapxmlData)
	{
		CData xmlData;
		TiXmlDocument doc;
		const char* errorMsg = doc.Parse(SoapxmlData);
		if (errorMsg)
		{
			LogError("SoapxmlData Parse failed: " << errorMsg);
			return "";
		}
		TiXmlElement* root = doc.RootElement();
		if (!root)
		{
			LogError("Failed to parse XML, no root element found." << SoapxmlData);
			return "";
		}
		// 获取Body元素
		TiXmlElement* bodyElement = root->FirstChildElement("soapenv:Body");
		if (!bodyElement)
		{
			bodyElement = root->FirstChildElement("SOAP-ENV:Body");
		}
		if (bodyElement)
		{
			// 获取invokeResponse元素
			TiXmlElement* invokeResponseElement = bodyElement->FirstChildElement("ns1:invoke");
			if (!invokeResponseElement)
			{
				invokeResponseElement = bodyElement->FirstChildElement("ns:invoke");
			}
			if (invokeResponseElement)
			{
				// 现在您可以访问或处理invokeResponse元素及其内容了
				// 查找invokeReturn元素  
				TiXmlElement* invokeReturnElement = invokeResponseElement->FirstChildElement("xmlData");
				if (!invokeReturnElement)
				{
					invokeReturnElement = invokeResponseElement->FirstChildElement("invoke");
				}
				if (!invokeReturnElement)
				{
					LogError("invokeReturn element not found." << SoapxmlData);
					return "";
				}
				// 获取invokeReturn元素的文本内容  
				std::string pXmlData = invokeReturnElement->GetText();
				if (!pXmlData.empty())
				{
					pXmlData = replace_all(pXmlData, "&lt;","<");
					pXmlData = replace_all(pXmlData, "&gt;",">");
					LogInfo("xmlData : " << pXmlData);
					xmlData = pXmlData;
				}
				else {
					LogError("No text content found in invokeReturn element." << SoapxmlData);
				}
			}
			else
			{
				LogError("invokeResponse element not found in SOAP Body." << SoapxmlData);
			}
		}
		else
		{
			LogError("SOAP Body element not found." << SoapxmlData);
		}
		return xmlData;
	}
}