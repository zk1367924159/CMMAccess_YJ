
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
			std::string clientIP = socket.peerAddress().host().toString();
			LogInfo("Client connected from: " << clientIP);
			// 处理客户端连接
			futures.push_back(std::async(std::launch::async, [socket, clientIP]() mutable {
				try
				{
					while (true)
					{
						std::vector<uint8_t> recvBuffer(1024); // 初始大小为1024
						int recvBytes = socket.receiveBytes(recvBuffer.data(), recvBuffer.size());

						// 检测客户端是否断开连接
						if (recvBytes <= 0)
						{
							LogInfo("Client disconnected: " << clientIP);
							break; // 退出循环，结束连接
						}
						// 处理接收到的数据
						if (recvBytes == (int)recvBuffer.size())
						{
							LogNotice("Data from " << clientIP << " is too large. Resizing buffer.");
							recvBuffer.resize(2 * recvBytes);
							continue;
						}
						if (recvBuffer.size() >= MAX_RECV_DATASIZE)
						{
							LogError("Data from " << clientIP << " exceeds maximum size.");
							continue;
						}
						std::vector<uint8_t> outBuffer;
						outBuffer.reserve(recvBytes);
						if (!TransData::UnPackageRecvData(recvBuffer, recvBytes, outBuffer))
						{
							std::string response = "UnPackageRecvData failed.";
							LogError("UnPackageRecvData failed for client: " << clientIP);
							socket.sendBytes(response.c_str(), response.length());
							continue;
						}
						LogInfo("Received " << recvBytes << " bytes from " << clientIP << ". OutBuffer size: " << outBuffer.size());
						if (!CMMAccess::instance()->writeDataToUart(outBuffer))
						{
							LogError("writeDataToUart failed for client: " << clientIP);
							std::string response = "writeDataToUart failed.";
							socket.sendBytes(response.c_str(), response.length());
							continue;
						}
					}
				}
				catch (const std::exception& e)
				{
					LogError("Exception for client " << clientIP << ": " << e.what());
				}
				// 关闭连接
				socket.close();
				LogInfo("Connection closed for client: " << clientIP);
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
		while (_running.load())
		{
			// 接收数据，尝试简单重传逻辑
			int recvBytes = 0;
			bool received = false;
			std::vector<uint8_t> recvBuffer(1024); // 初始大小为1024  
			SocketAddress senderAddr;
			try
			{
				while (true)
				{
					recvBytes = _socket->receiveFrom(recvBuffer.data(), recvBuffer.size(), senderAddr);
					if (recvBytes <= 0)
					{
						received = false;
						LogError("recv ip: " << senderAddr.host() << " data recv error. please check data retry.");
						break;
					}
					else if (recvBytes == (int)recvBuffer.size())
					{
						// 缓冲区可能不足以容纳所有数据，增加缓冲区大小  
						received = false;
						recvBuffer.resize(recvBytes * 2); // 示例：加倍缓冲区大小  
						break;
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
					//std::string response = "Failed to receive data after retries.";
					//m_ServerSocket.sendTo(response.c_str(), response.length(), senderAddr);
					LogError("Failed to receive data after retries.");
					continue;
				}
				std::vector<uint8_t> outBuffer;
				outBuffer.reserve(recvBytes);
				if (!TransData::UnPackageRecvData(recvBuffer, recvBytes, outBuffer))
				{
					std::string response = "UnPackageRecvData failed.";
					_socket->sendTo(response.c_str(), response.length(), senderAddr);
					LogError("UnPackageRecvData return false.");
					continue;
				}
				LogInfo("recvBytes: " << recvBytes << " outBuffer size:" << outBuffer.size());
				if (!CMMAccess::instance()->writeDataToUart(outBuffer))
				{
					std::string response = "writeDataToUart failed.";
					_socket->sendTo(response.c_str(), response.length(), senderAddr);
					LogError("writeDataToUart false.");
					continue;
				}
				//std::string response = "writeDataToUart sucess.";
				//m_ServerSocket.sendTo(response.c_str(), response.length(), senderAddr);
			}
			catch (Poco::Exception& exc)
			{
				LogError("Exception msg: " << exc.displayText());
			}
		}
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
	LogInfo("DoorServerManger start " << type << " port: " << port);
	try
	{
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
			LogInfo("Start server failed ");
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






