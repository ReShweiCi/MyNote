#pragma once
/*
* A SDL_Net based TCP wrapper to solve sticky packet problem
* 2023.09.27 by ReShweiCi
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <string>
#include <spdlog/spdlog.h>
#include <vector>
#define BUFFER_SIZE 1024
#define HEADER_LENGTH sizeof(unsigned int)

class TCPWrapper
{
public:
	TCPsocket m_socket;
	// create a server
	TCPWrapper(Uint16 port)
	{
		IPaddress ip;
		if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
			spdlog::error("Failed to resolve host, {}", SDLNet_GetError());
		m_socket = SDLNet_TCP_Open(&ip);
		if (m_socket)
			spdlog::info("Success to create a server on port {}", port);
		else
			spdlog::error("Failed to start server, {}", SDLNet_GetError());
	}
	// create a client
	TCPWrapper(const char* host, Uint16 port)
	{
		IPaddress ip;
		if (SDLNet_ResolveHost(&ip, host, port) == -1)
			spdlog::error("Failed to resolve host, {}", SDLNet_GetError());
		m_socket = SDLNet_TCP_Open(&ip);
	}
	~TCPWrapper()
	{
		while (!m_rawDataList.empty())
		{
			char* raw_data = m_rawDataList.front();
			free(raw_data);
			m_rawDataList.erase(m_rawDataList.begin());
		}
		SDLNet_TCP_Close(m_socket);
	}
	/*to solve TCP sticky packet problem*/
	// construct the packet and send
	int Send(TCPsocket& sock, const char* raw_data, unsigned int lenth)
	{
		char* package = (char*)malloc(lenth + sizeof(lenth));
		unsigned int* len = (unsigned int*)package;
		*len = lenth;
		memcpy(package + HEADER_LENGTH, raw_data, lenth);
		int data_size = SDLNet_TCP_Send(sock, package, lenth + sizeof(lenth));
		if (data_size == -1)
		{
			spdlog::error("Failed to send socket data, {}", SDLNet_GetError());
			free(package);
			return -1;
		}
		else
		{
			free(package);
			return data_size;
		}
	}

	// recieve the packet and push the string into pool
	void Recieve(TCPsocket& sock)
	{
		// current position stands for the byte which is going to be processed
		int currentPos = 0;
		unsigned int* len = (unsigned int*)malloc(HEADER_LENGTH);
		int data_size = SDLNet_TCP_Recv(sock, buffer, BUFFER_SIZE);
		// if there is an error
		if (data_size == -1)
		{
			spdlog::error("Failed to recieve socket data, {}\n", SDLNet_GetError());
			free(len);
			return;
		}
		// read the header to load length
		memcpy(len, buffer, HEADER_LENGTH);
		// process all the full packet until remaining part is incomplete
		while (currentPos + HEADER_LENGTH + *len <= BUFFER_SIZE && currentPos != data_size)
		{
			currentPos += HEADER_LENGTH + *len;
			UnpackCompletePacket(buffer, *len);
		}
		// if all the packet have been processed
		if (currentPos == data_size)
		{
			free(len);
			return;
		}
		// if the last packet was cut in the body
		if (BUFFER_SIZE - currentPos > HEADER_LENGTH)
		{
			//read the length
			unsigned int* len = (unsigned int*)malloc(HEADER_LENGTH);
			memcpy(len, buffer + currentPos, HEADER_LENGTH);
			// allocate temp buffer to store the full packet
			char* tempBuffer = (char*)malloc(HEADER_LENGTH + *len);
			memcpy(tempBuffer, buffer + currentPos, BUFFER_SIZE - currentPos);
			data_size = SDLNet_TCP_Recv(sock, tempBuffer + BUFFER_SIZE - currentPos, *len + HEADER_LENGTH - (BUFFER_SIZE - currentPos));
			UnpackCompletePacket(tempBuffer, *len);
			free(tempBuffer);
			free(len);
		}
		// if the last packet was cut in the header
		else
		{
			// copy part of header to the buffer beginning
			memcpy(buffer, buffer + currentPos, BUFFER_SIZE - currentPos);
			// recieve the remaining part of header
			data_size = SDLNet_TCP_Recv(sock, buffer + BUFFER_SIZE - currentPos, HEADER_LENGTH - (BUFFER_SIZE - currentPos));
			unsigned int* len = (unsigned int*)malloc(HEADER_LENGTH);
			memcpy(len, buffer, HEADER_LENGTH);
			char* tempBuffer = (char*)malloc(HEADER_LENGTH + *len);
			// copy the header to the temp buffer
			memcpy(tempBuffer, len, HEADER_LENGTH);
			// recieve the body to the temp buffer
			data_size = SDLNet_TCP_Recv(sock, tempBuffer + HEADER_LENGTH, *len);
			UnpackCompletePacket(tempBuffer, *len);
			free(tempBuffer);
			free(len);
		}
	}
	
	// try to get data from the data pool
	bool GetData(std::string* data)
	{
		if (m_rawDataList.empty())
			return false;
		char* str = m_rawDataList.front();
		*data = str;
		free(str);
		m_rawDataList.erase(m_rawDataList.begin());
		return true;
	}
	
private:
	char buffer[BUFFER_SIZE];
	// remember to call free();
	std::vector<char*> m_rawDataList = {};
	void UnpackCompletePacket(char* source, unsigned int &len)
	{
		char* str = (char*)malloc(len);
		memcpy(str, source + HEADER_LENGTH, len);
		m_rawDataList.push_back(str);
	}
};