//canyon 2019 09 06

#include "DoorClient.h"
#include "TransData.h"
#include "CLog.h"
#include "openssl/sha.h"
//#include "openssl/openssl-1.1.1d/crypto/include/internal/sm3.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "Poco/HMACEngine.h"
#include "Poco/StreamCopier.h"


using namespace Poco::Net;



namespace CMM
{

	DoorClient::DoorClient()
	{

	}

	DoorClient::~DoorClient()
	{

	}

	void DoorClient::Start()
	{
		//m_pUser = url;
	}

	void DoorClient::Stop()
	{
		m_tcpManager.reset();
	}

	int DoorClient::receiveTCPData(Poco::Net::SocketStream& stream)
	{
		try
		{
			std::vector<char> recvBuffer;
			char ch;
			// 循环读取数据直到遇到换行符或发生错误
			while ((ch =stream.get()) != EOF )
			{
				if (ch == '\n') {
					break; // 假设换行符是数据的结束标记
				}
				recvBuffer.push_back(ch);
			}
			// 检查是否因为流结束而退出循环（这里不太可能，因为TCP是流式协议）
			if (!stream.good())
			{
				LogError("Failed to read data from TCP socket.");
				return -3;
			}
			// 将接收到的数据转换为字符串并打印
			std::string receivedData(recvBuffer.begin(), recvBuffer.end());
			LogInfo("Received TCP data: " << receivedData);
		}
		catch (Poco::Exception& exc)
		{
			LogError("TCP receive exception: " << exc.displayText());
			return -4; // 发生异常
		}
		return 0;
	}

	int DoorClient::receiveUDPData(Poco::Net::DatagramSocket& socket)
	{
		try
		{
			bool received = false;
			int recvBytes = 0;
			int nCount = 0;
			std::vector<uint8_t> recvBuffer(1024); // 初始大小为1024  
			while (true)
			{
				SocketAddress senderAddr;
				recvBytes = socket.receiveFrom(recvBuffer.data(), recvBuffer.size(), senderAddr);
				if (recvBytes <= 0)
				{
					nCount++;
				}
				else if (nCount > 3)
				{
					received = false;
					LogError("recv ip: " << senderAddr.host() << " data recv error. please check data retry.");
					break;
				}
				else if (recvBytes == (int)recvBuffer.size())
				{
					// 缓冲区可能不足以容纳所有数据，增加缓冲区大小  
					recvBuffer.resize(recvBuffer.size() * 2); // 示例：加倍缓冲区大小  
				}
				else if (recvBuffer.size() >= MAX_RECV_DATASIZE)
				{
					received = false;
					LogError("recv ip: " << senderAddr.host() << " data is so Larger. please check data retry.");
					break;
				}
				else
				{
					received = true;
					break; // 收到小于缓冲区大小的数据，或发生错误  
				}
			}
			if (!received)
			{
				LogError("Failed to receive data after retries.");
				return -2;
			}
			std::string buffer(recvBuffer.begin(), recvBuffer.end());
			LogInfo("recv server response:" << buffer.c_str());
		}
		catch (Poco::Exception& exc)
		{
			LogError("Exception msg: " << exc.displayText());
			return -4;
		}
		return 0;
	}

	int DoorClient::SendData(const char* url, CData protocolType, std::vector<uint8_t>& uartData)
	{
		Poco::URI uri(url);
		std::string serverAddress = uri.getHost();
		int serverPort = uri.getPort();
		LogInfo("SEND data to serverAddress " << serverAddress << " and serverPort:" << serverPort);
		if (protocolType == "udp")
		{
			try
			{
				SocketAddress serverAddr(SocketAddress::IPv4, serverAddress, serverPort);
				auto& socket = SingletonSocket::instance().getSocket();
				socket.setReceiveTimeout(Poco::Timespan(5, 0));
				std::vector<uint8_t> sendData = TransData::PackageSendData(uartData);
				int sentBytes = socket.sendTo(sendData.data(), sendData.size(), serverAddr);
				if (sentBytes < 0)
				{
					LogError("send msg error: " << sentBytes);
					return -1;
				}
				receiveUDPData(socket);
			}
			catch (Poco::Exception& exc)
			{
				LogError("Exception msg: " << exc.displayText());
				return -1;
			}
		}
		else if (protocolType == "tcp")
		{
			try
			{
				if (!m_tcpManager)
				{
					m_tcpManager = new TCPSocketManager(serverAddress, serverPort);
				}
				std::vector<uint8_t> sendData = TransData::PackageSendData(uartData);
				m_tcpManager->getStream().write(reinterpret_cast<const char*>(sendData.data()), sendData.size());
				m_tcpManager->getStream().flush();
				receiveTCPData(m_tcpManager->getStream());
			}
			catch (Poco::Exception& exc)
			{
				LogError("Exception msg: " << exc.displayText());
				return -1;
			}
		}
		else
		{
			return -1;
		}
		return 0;
	}

	int DoorClient::SendHeart(const char* url, CData protocolType)
	{
		Poco::URI uri(url);
		std::string serverAddress = uri.getHost();
		int serverPort = uri.getPort();
		LogInfo("SEND data to serverAddress " << serverAddress << " and serverPort:" << serverPort);
		if (protocolType == "udp")
		{
			try
			{
				SocketAddress serverAddr(SocketAddress::IPv4, serverAddress, serverPort);
				auto& socket = SingletonSocket::instance().getSocket();
				socket.setReceiveTimeout(Poco::Timespan(5, 0));
				std::vector<uint8_t> sendData = TransData::PackageSendHeart();
				int sentBytes = socket.sendTo(sendData.data(), sendData.size(), serverAddr);
				if (sentBytes < 0)
				{
					LogError("send msg error: " << sentBytes);
					return -1;
				}
				return receiveUDPData(socket);
			}
			catch (Poco::Exception& exc)
			{
				LogError("Exception msg: " << exc.displayText());
				return -1;
			}
		}
		else if (protocolType == "tcp")
		{
			try
			{
				if (!m_tcpManager)
				{
					m_tcpManager = new TCPSocketManager(serverAddress, serverPort);
				}
				std::vector<uint8_t> sendData = TransData::PackageSendHeart();
				m_tcpManager->getStream().write(reinterpret_cast<const char*>(sendData.data()), sendData.size());
				m_tcpManager->getStream().flush();
				return receiveTCPData(m_tcpManager->getStream());
			}
			catch (Poco::Exception& exc)
			{
				LogError("Exception msg: " << exc.displayText());
				return -1;
			}
		}
		else
		{
			return -1;
		}
		return 0;
	}

}

