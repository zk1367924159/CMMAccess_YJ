
#include "DoorServerManger.h"

#include "CMMAccess.h"

using namespace Poco::Net;

namespace CMM
{

	void TcpServer::run()
	{
		std::vector<std::future<void>> futures;
		while (_running.load())
		{
			StreamSocket socket = _socket->acceptConnection();
			// 处理客户端连接
			futures.push_back(std::async(std::launch::async, [socket]() mutable {
				char buffer[1024];
				int n = socket.receiveBytes(buffer, sizeof(buffer));
				if (n > 0) {
					std::string message(buffer, n);
					LogInfo("Received: " << message);
				}
				socket.close();
				}));
			// 清理已完成的任务
			futures.erase(std::remove_if(futures.begin(), futures.end(), [](std::future<void>& future) {
				return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
				}), futures.end());
		}
		// 等待所有任务完成
		for (auto& future : futures)
		{
			future.wait();
		}
	}

	void TcpServer::stop()
	{
		_running.store(false);
		// Optionally, close the socket here if needed
		// _socket->close(); // Note: For UDP, this might not be necessary or sufficient
	}

	void UdpServer::run()
	{
		std::vector<uint8_t> recvBuffer(1024); // 初始大小为1024  
		while (_running.load())
		{
			SocketAddress senderAddr;
			int recvBytes = _socket->receiveFrom(recvBuffer.data(), recvBuffer.size(), senderAddr);
			if (recvBytes <= 0)
			{
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
				LogError("recv ip: " << senderAddr.host() << " data is so Larger. please check data retry.");
				break;
			}
			else
			{
				break; // 收到小于缓冲区大小的数据，或发生错误  
			}

			std::vector<uint8_t> outBuffer;
			outBuffer.reserve(recvBytes);
			if (!TransData::UnPackageRecvData(recvBuffer, recvBytes, outBuffer))
			{
				std::string response = "UnPackageRecvData failed.";
				//m_ServerSocket.sendTo(response.c_str(), response.length(), senderAddr);
				LogError("UnPackageRecvData return false.");
				continue;
			}
			LogInfo("recvBytes: " << recvBytes << " outBuffer size:" << outBuffer.size());
			if (!CMMAccess::instance()->writeDataToUart(outBuffer))
			{
				std::string response = "writeDataToUart failed.";
				//m_ServerSocket.sendTo(response.c_str(), response.length(), senderAddr);
				LogError("writeDataToUart false.");
				continue;
			}
			std::string response = "writeDataToUart sucess.";
			//m_ServerSocket.sendTo(response.c_str(), response.length(), senderAddr);
		}
	}

	void UdpServer::stop()
	{
		_running.store(false);
		// Optionally, close the socket here if needed
		// _socket->close(); // Note: For UDP, this might not be necessary or sufficient
	}

	void DoorServerManger::Start(std::string type, unsigned short port)
	{
		try
		{
			std::lock_guard<std::mutex> lock(_mutex);
			stopServer();
			_currentServer = nullptr;
			if (type == "tcp")
			{
				_currentServer = new TcpServer(port);
				LogInfo("Start tcp port: " << port);
			}
			else if (type == "udp")
			{
				// Note: UDP server handling is more complex because you typically don't create a new socket per client
				// For simplicity, we'll omit UDP server implementation here
				// But you could create a separate UDP server class that runs in its own thread and listens continuously
				_currentServer = new UdpServer(port);
				LogInfo("Start udp port: " << port);
			}
			else
			{

				return;

			}
		}
		catch (const std::exception& e)
		{
			LogError("Failed to start server: " << e.what());
		}

	}
	void DoorServerManger::stopServer()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_currentServer) {
			_currentServer->stop();
			_currentServer.reset();
		}
	}
}





