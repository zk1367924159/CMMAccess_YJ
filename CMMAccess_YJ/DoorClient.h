//canyon 2019 09 06

#pragma once

#include <stdio.h>
#include "Poco/SharedPtr.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Timestamp.h"
#include "Data.h"


#include <Poco/Net/StreamSocket.h>    // TCP
#include <Poco/Net/SocketStream.h>    // TCP 发送和接收数据
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h" 
#include "Poco/Net/SocketAddressImpl.h"
#include "Poco/URI.h"
#include "Poco/DigestStream.h"

using namespace Poco::Net;


namespace CMM
{

	class SingletonSocket {
	public:
		static SingletonSocket& instance() {
			static SingletonSocket singleton; // 静态局部变量，线程安全初始化
			return singleton;
		}

		DatagramSocket& getSocket() {
			return socket_;
		}

	private:
		DatagramSocket socket_;

		SingletonSocket() {
			socket_.bind(SocketAddress(SocketAddress::IPv4, "0.0.0.0", 0));
			socket_.setSendTimeout(Poco::Timespan(3, 0));
			socket_.setReceiveTimeout(Poco::Timespan(3, 0));
		}

		~SingletonSocket() {
			socket_.close();
		}

		SingletonSocket(const SingletonSocket&) = delete;
		SingletonSocket& operator=(const SingletonSocket&) = delete;
	};

	class TCPSocketManager 
	{
	public:
		TCPSocketManager(const std::string& serverAddress, int serverPort, int timeoutSeconds = 3)
			: tcpSocket_(Poco::Net::SocketAddress(serverAddress, serverPort)), stream_(tcpSocket_) 
		{
			Poco::Timespan timeout(timeoutSeconds, 0); // 动态设置超时时间
			tcpSocket_.setSendTimeout(timeout);
			tcpSocket_.setReceiveTimeout(timeout);
		}

		~TCPSocketManager() = default;

		Poco::Net::SocketStream& getStream() {
			return stream_;
		}

		bool isConnected() {
			Poco::Timespan timeout(0, 0); // 非阻塞模式
			return tcpSocket_.poll(timeout, Poco::Net::Socket::SELECT_WRITE | Poco::Net::Socket::SELECT_ERROR);
		}

	private:
		Poco::Net::StreamSocket tcpSocket_;
		Poco::Net::SocketStream stream_;

		// 禁止拷贝和赋值
		TCPSocketManager(const TCPSocketManager&) = delete;
		TCPSocketManager& operator=(const TCPSocketManager&) = delete;
	};

	class DoorClient 
	{

	public:
		DoorClient();
		~DoorClient();
		void Start();
		void Stop();
		int receiveTCPData(Poco::Net::SocketStream& stream);
		int receiveUDPData(Poco::Net::DatagramSocket& scoket);
		/*
		* 发送串口数据 返回1成功 -2超时 其他失败
		*/
		int SendData(const char* url,CData protocolType, std::vector<uint8_t>& uartData);
		/*
		* 发送心跳
		*/
		int SendHeart(const char* url, CData protocolType);
	private:
		std::string m_pUser;
		Poco::SharedPtr<TCPSocketManager> m_tcpManager;
	};
		
}
