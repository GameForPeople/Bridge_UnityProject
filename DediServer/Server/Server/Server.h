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

#include "CommunicationProtocol.h"

using namespace std;

#define SERVER_PORT 9000
#define BUF_SIZE 512

// For ExternalIP
#define EXTERNALIP_FINDER_URL "http://checkip.dyndns.org/"
#define TITLE_PARSER "<body>Current IP Address: "

// For Login
#define MAX_ID_LEN 12

struct SOCKETINFO
{
	OVERLAPPED overlapped;	// OVERLAPPED ����ü
	WSABUF wsabuf;
	SOCKET sock;
	char buf[BUF_SIZE + 1];
	int dataSize;
	bool isRecvTrue; // �̰� Ʈ��� ���� Ÿ�̹�, flase�� �ִ� Ÿ�̹�
	int bufferProtocol; // �̰Ŵ� ���� �� ���� ���ؾ��ϴ��� �����س��°ž� �� ������...

	BaseStruct* dataBuffer;	// ����� ģ��..
};

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
	__inline CUserData(std::string InID, const int InPW) : m_id(InID), m_pw(InPW), m_winCount(0), m_loseCount(0), m_money(0), m_isLogin(true)
	{ }; // ȸ�������̶�, �α��ε� �ٷ� On�������!

	__inline CUserData(const std::string InID, const int InPW, const int InWinCount, const int InloseCount, const int InMoney)
		: m_id(InID), m_pw(InPW), m_winCount(InWinCount), m_loseCount(InloseCount) , m_money(InMoney), m_isLogin(false)
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


