//canyon 2019 09 06

#pragma once
#include <mutex>
#include <iostream>
#include <memory>
#include <atomic>
#include <future>
#include <vector>
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/SharedPtr.h"
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "CLog.h"
#include "TransData.h"

using namespace Poco::Net;
namespace CMM
{
	class CMMAccess;

	class ServerBase : public Poco::Runnable 
	{
	public:
		virtual ~ServerBase() = default;
		virtual void stop() = 0;
	};

	class TcpServer : public ServerBase 
	{
	public:
		TcpServer(unsigned short port) : _socket(new ServerSocket(port)), _running(true)
		{
			_thread.start(*this);
		}

		~TcpServer() override
		{
			stop();
			_thread.join();
			_socket->close();
		}
		virtual void run();
		void stop();
	private:
		Poco::SharedPtr<ServerSocket>  _socket;
		Poco::Thread _thread;
		std::atomic<bool> _running;
	};

	class UdpServer : public ServerBase
	{
	public:
		UdpServer(unsigned short port) : _socket(new DatagramSocket(SocketAddress("0.0.0.0", port))), _running(true)
		{
			_thread.start(*this);
		}

		~UdpServer() override
		{
			stop();
			_thread.join();
			_socket->close();
		}

		virtual void run();
		void stop();
	private:
		Poco::SharedPtr<DatagramSocket> _socket;
		Poco::Thread _thread;
		std::atomic<bool> _running;
	};

	class DoorServerManger {
	public:
		DoorServerManger() : _currentServer(nullptr), _stopRequested(false) {}
		~DoorServerManger() {
			stopServer();
		}
		void Start(std::string type, unsigned short port);
		void stopServer();

	private:
		Poco::SharedPtr<ServerBase> _currentServer;
		std::mutex _mutex;
		std::atomic<bool> _stopRequested;
	};
}