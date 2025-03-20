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
			const size_t bufferSize = 1024; // 每次读取的缓冲区大小
			const size_t maxMessageSize = 65536; // 单条消息的最大长度（64KB）
			std::vector<char> recvBuffer;
			char buffer[bufferSize];
			while (true)
			{
				// 批量读取数据
				stream.read(buffer, bufferSize);
				std::streamsize bytesRead = stream.gcount();

				if (bytesRead > 0)
				{
					// 将读取的数据添加到接收缓冲区
					recvBuffer.insert(recvBuffer.end(), buffer, buffer + bytesRead);

					// 检查是否收到结束标识
					auto it = std::find(recvBuffer.begin(), recvBuffer.end(), 0xFE);
					if (it != recvBuffer.end())
					{
						// 找到换行符，提取完整消息
						std::string receivedData(recvBuffer.begin(), it);
						LogInfo("Received TCP data: " << receivedData);

						// 移除已处理的数据
						recvBuffer.erase(recvBuffer.begin(), it + 1);
						return 0; // 成功
					}

					// 检查消息是否过长
					if (recvBuffer.size() > maxMessageSize)
					{
						LogError("Message size exceeds maximum limit.");
						return -2; // 消息过长
					}
				}
				else if (bytesRead == 0)
				{
					// 流结束（对方关闭连接）
					LogInfo("Connection closed by remote peer.");
					return -1; // 连接关闭
				}
				else
				{
					// 读取失败
					LogError("Failed to read data from TCP socket.");
					return -3; // 读取失败
				}
			}
		}
		catch (Poco::Exception& exc)
		{
			LogError("TCP receive exception: " << exc.displayText());
			return -4; // 发生异常
		}
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
				std::vector<uint8_t> sendData = TransData::PackageSendData(uartData);
				int sentBytes = socket.sendTo(sendData.data(), sendData.size(), serverAddr);
				if (sentBytes < 0)
				{
					LogError("send msg error: " << sentBytes);
					return -1;
				}
				return receiveUDPData(socket);
			}
			catch (Poco::TimeoutException& exc) {
				LogError("Timeout error: " << exc.displayText());
				return -2; // 超时错误
			}
			catch (Poco::Exception& exc) {
				LogError("General error: " << exc.displayText());
				return -1; // 其他错误
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
				// 检查连接状态
				if (!m_tcpManager->isConnected())
				{
					LogError("TCP connection is not established.");
					return -1;
				}
				std::vector<uint8_t> sendData = TransData::PackageSendData(uartData);
				m_tcpManager->getStream().write(reinterpret_cast<const char*>(sendData.data()), sendData.size());
				m_tcpManager->getStream().flush();
				return receiveTCPData(m_tcpManager->getStream());
			}
			catch (Poco::TimeoutException& exc) {
				LogError("Timeout error: " << exc.displayText());
				return -2; // 超时错误
			}
			catch (Poco::Exception& exc) {
				LogError("General error: " << exc.displayText());
				return -1; // 其他错误
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
		LogInfo("SEND heart to serverAddress " << serverAddress << " and serverPort:" << serverPort);
		if (protocolType == "udp")
		{
			try
			{
				SocketAddress serverAddr(SocketAddress::IPv4, serverAddress, serverPort);
				auto& socket = SingletonSocket::instance().getSocket();
				std::vector<uint8_t> sendData = TransData::PackageSendHeart();
				socket.sendTo(sendData.data(), sendData.size(), serverAddr);
				return 0;
			}
			catch (Poco::TimeoutException& exc) {
				LogError("Timeout error: " << exc.displayText());
				return -2; // 超时错误
			}
			catch (Poco::Exception& exc) {
				LogError("General error: " << exc.displayText());
				return -1; // 其他错误
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
				// 检查连接状态
				if (!m_tcpManager->isConnected())
				{
					LogError("TCP connection is not established.");
					return -1;
				}
				std::vector<uint8_t> sendData = TransData::PackageSendHeart();
				Poco::Timespan timeout(3, 0); // 5秒超时
				m_tcpManager->getStream().socket().setSendTimeout(timeout);
				m_tcpManager->getStream().write(reinterpret_cast<const char*>(sendData.data()), sendData.size());
				m_tcpManager->getStream().flush();
				return 0;// receiveTCPData(m_tcpManager->getStream());
			}
			catch (Poco::TimeoutException& exc) {
				LogError("Timeout error: " << exc.displayText());
				return -2; // 超时错误
			}
			catch (Poco::Exception& exc) {
				LogError("General error: " << exc.displayText());
				return -1; // 其他错误
			}
		}
		else
		{
			return -1;
		}
		return 0;
	}

}

