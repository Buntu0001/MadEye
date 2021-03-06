#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <thread>
#include <ctime>

class Flooder
{
public:
	Flooder(LPSTR ip, DWORD port, DWORD duration);
	~Flooder();
	void Worker();
	std::thread WorkerThread();

	LPSTR ip_;
	DWORD port_;
	DWORD duration_;

	WSADATA wsadata_;
	SOCKET flood_socket;

};