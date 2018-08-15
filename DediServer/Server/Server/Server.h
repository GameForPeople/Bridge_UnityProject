#pragma once

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "wininet.lib")

#include <WinSock2.h>
#include <iostream>
#include <cstdlib>

#include <fstream>

#include <vector>
#include <string>

// For ExternalIP
#include "wininet.h"
#include <tchar.h>

using namespace std;

#define SERVER_PORT 9000
#define BUF_SIZE 512

// For ExternalIP
#define EXTERNALIP_FINDER_URL "http://checkip.dyndns.org/"
#define TITLE_PARSER "<body>Current IP Address: "

// For Login
#define MAX_ID_LEN 12

class CUserData {
	//basic Data
	std::string m_id{};
	int m_pw{};
	int m_winCount{};
	int m_loseCount{};
	int m_money{};

	//Game use Data
	bool m_isLogin{ false };
	IN_ADDR m_userAddr{};

public:
	__inline CUserData() {};
	__inline CUserData(std::string InID, const int InPW) : m_id(InID), m_pw(InPW), m_winCount(0), m_loseCount(0), m_money(0)
	{ };
	__inline CUserData(const std::string InID, const int InPW, const int InWinCount, const int InloseCount, const int InMoney)
		: m_id(InID), m_pw(InPW), m_winCount(InWinCount), m_loseCount(InloseCount) , m_money(InMoney)
	{ };

	~CUserData() {};

public:
	__inline void	SetIPAddress(IN_ADDR& inputAddress) { m_userAddr = inputAddress; m_isLogin = true; }

	__inline void	PrintUserData() 
	{ 
		std::cout << m_id << "  " << m_pw << "  " << m_winCount << "  " << m_loseCount << std::endl;
	}
	
	__inline string	GetID() { return m_id; }
	__inline int	GetPW() { return m_pw; }
	__inline int	GetWinCount() { return m_winCount; }
	__inline int	GetLoseCount() { return m_loseCount; }
	__inline int	GetMoney() { return m_money; }

	__inline void	SetWinOrLose(int value) {
		if (value == 1) { m_winCount++; }
		else if (value == 2) { m_loseCount++; }
		return;
	}

	__inline bool	GetIsLogin() { return m_isLogin; }
	__inline void	SetIsLogin(bool bValue) { m_isLogin = bValue; }
};

struct SOCKETINFO
{
	OVERLAPPED overlapped;	// OVERLAPPED ����ü
	SOCKET sock;
	char buf[BUF_SIZE + 1];
	int recvBytes{};
	int sendBytes{};
	WSABUF wsabuf;
};

void err_quit(char *msg) 
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	MessageBox(NULL, (LPTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
};

void err_display(char *msg) 
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	printf(" [%s]  %S", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
};

/*
IOCompletionQueue
FIFO Queue.������� �Ϸ�� �۾����� ����. thread���� �� queue���� �۾��� ������ �����Ѵ�.

WaitingThreadQueue
LIFO Queue.(�� �̸��� queue��) �۾� ������� thread���� ����ִ�. ���� IO�� �Ϸ�Ǿ��ٸ� �� queue���� thread�� �ϳ� ������ ����Ѵ�.

ReleaseThreadList
���� �۾��� �����ϰ� �ִ� thread���� ����.
���� �������� thread�� �����ؾ� �Ѵٸ� �ش� thread�� PauseThreadList�� ������ WaitingThreadQueue���� ���ο� thread�� �ϳ� �����ͼ� ����Ѵ�. �� ������ ���μ����� ������ ���� thread�� �̸� ����� ���� ��.

PauseThreadList
� ����(�Ӱ� ���� ��)���� ���� �Ͻ������� thread���� ����.
���� �Ͻ������� Ǯ������ ReleaseThreadList�� �� ���ִٸ� �ٷ� ReleaseThreadList�� ������ �ʰ� ����Ѵ�.
*/

DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retVal{};
	HANDLE hcp = (HANDLE)arg;

	int recvType{};
	int sendType{};

	while (7)
	{
		//�񵿱� ����� ��ٸ���
		DWORD cbTransferred;
		SOCKET clientSocket;
		SOCKETINFO *ptr;

		// ����� �Ϸ� ��Ʈ�� ����� ����� ó���ϱ� ���� �Լ� // ��� ���°� ��
		retVal = GetQueuedCompletionStatus(
			hcp, //����� �Ϸ� ��Ʈ �ڵ�
			&cbTransferred, //�񵿱� ����� �۾�����, ���۵� ����Ʈ ���� ���⿡ ����ȴ�.
			(LPDWORD)&clientSocket, //�Լ� ȣ�� �� ������ ����° ����(32��Ʈ) �� ���⿡ ����ȴ�.
			(LPOVERLAPPED *)&ptr, //Overlapped ����ü�� �ּҰ�
			INFINITE // ��� �ð� -> ��� ���� ���Ѵ�
		);

		// �Ҵ���� ���� ��! Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientAddr;
		int addrLength = sizeof(clientAddr);
		getpeername(ptr->sock, (SOCKADDR *)&clientAddr, &addrLength);

		//�񵿱� ����� ��� Ȯ�� // �ƹ��͵� �Ⱥ��� ����, �ش� Ŭ���̾�Ʈ ���ӿ� ������ ��������� �Ǵ�, �ݾƹ����ڴ�!
		if (retVal == 0 || cbTransferred == 0)
		{
			std::cout << "DEBUG - A" << std::endl;

			if (retVal == 0)
			{
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &temp1, FALSE, &temp2);
				err_display((char *)"WSAGetOverlappedResult()");
			}
			closesocket(ptr->sock);

			printf("[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� =%s, ��Ʈ ��ȣ =%d\n",
				inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			delete ptr;
			continue;
		}

		// ������ ���۷� ����
		if (ptr->recvBytes == 0)
		{
			std::cout << "DEBUG - B" << std::endl;

			//cbTransferred�� ���� �������� ũ�⸦ ����!! --> �� ����(����, ����, ����)�� 1byte, (�ѱ�)2byte
			ptr->recvBytes = cbTransferred;
			ptr->sendBytes = 0;


			// ���� ������ ���
			ptr->buf[ptr->recvBytes] = '\0';
			//printf(" [TCP %s :%d] %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), ptr->buf);
			std::cout << "[Debug] : ���۵� Size : "<< cbTransferred << "  ���� :  " << ptr->buf << std::endl;
		}
		else
		{
			std::cout << "DEBUG - C" << std::endl;

			ptr->sendBytes += cbTransferred;

		}

		if (ptr->recvBytes > ptr->sendBytes) 
		{
			std::cout << "DEBUG - D" << std::endl;

			// ������ ������
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf + ptr->sendBytes;
			ptr->wsabuf.len = ptr->recvBytes - ptr->sendBytes;

			DWORD sendBytes;
			retVal = WSASend(
				ptr->sock, // Ŭ���̾�Ʈ ����
				&ptr->wsabuf, // ���� ������ ������ ������
				1, // ������ �Է� ������ ����
				&sendBytes, // Send ����Ʈ ��...?
				0, // ??????????????
				&ptr->overlapped, // overlapped����ü�� ������
				NULL // IOCP������ ������� �����Ƿ� NULL, nullptr�Ѱܵ� ����
			);

			if (retVal == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					err_display((char *)"WSASend()");
				}
				continue;
			}


		}
		else {
			ptr->recvBytes = 0;

			//������ �ޱ�
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUF_SIZE;

			DWORD recvBytes;
			DWORD flags{0};
			retVal = WSARecv(
				ptr->sock, // Ŭ���̾�Ʈ ����
				&ptr->wsabuf, // ���� ������ ������ ������
				1, // ������ �Է� ������ ����
				&recvBytes, // recv ��� ���� ����Ʈ ��, IOCP������ �񵿱� ������� ������� �����Ƿ� nullPtr�� �Ѱܵ� ����
				&flags,  // recv�� ���� �÷���
				&ptr->overlapped, // overlapped����ü�� ������
				NULL // IOCP������ ������� �����Ƿ� NULL, nullptr�Ѱܵ� ����
			);

			if (retVal == SOCKET_ERROR) 
			{
				if (WSAGetLastError() != WSA_IO_PENDING) 
				{
					err_display((char *)"WSARecv()");
				}
				continue;
			}

			std::cout << "DEBUG - E" << std::endl;

		}
	}
};
 
int GetExternalIP(char *ip)
{
	HINTERNET hInternet, hFile;
	DWORD rSize;
	char buffer[256] = { 0 };

	hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (NULL == hInternet)
		return 0;

	hFile = InternetOpenUrl(hInternet, EXTERNALIP_FINDER_URL, NULL, 0, INTERNET_FLAG_RELOAD, 0);

	if (hFile)
	{
		InternetReadFile(hFile, &buffer, sizeof(buffer), &rSize);
		buffer[rSize] = '\0';

		int nShift = _tcslen(TITLE_PARSER);
		std::string strHTML = buffer;
		std::string::size_type nIdx = strHTML.find(TITLE_PARSER);
		strHTML.erase(strHTML.begin(), strHTML.begin() + nIdx + nShift);
		nIdx = strHTML.find("</body>");
		strHTML.erase(strHTML.begin() + nIdx, strHTML.end());

		_tcscpy(ip, strHTML.c_str());
		InternetCloseHandle(hFile);
		InternetCloseHandle(hInternet);

		return _tcslen(ip);
	}

	return 0;
}


