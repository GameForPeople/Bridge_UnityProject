#pragma once

#include "../stdafx.h"

#include "../Protocol/CommunicationProtocol.h"
#include "../UserData/UserData.h"
#include "../GameRoom/GameRoom.h"


#define		SERVER_PORT		9000
#define		BUF_SIZE		100

// For ExternalIP
#define		EXTERNALIP_FINDER_URL	"http://checkip.dyndns.org/"
#define		TITLE_PARSER			"<body>Current IP Address: "

// For Login
#define		MAX_ID_LEN		12

//#define SERVER_DEBUG_LOG_PRINT

struct SOCKETINFO
{
	OVERLAPPED overlapped;	// OVERLAPPED ����ü
	WSABUF wsabuf;
	SOCKET sock;
	char buf[BUF_SIZE + 1];
	int dataSize;
	bool isRecvTrue; // �̰� Ʈ��� ���� Ÿ�̹�, flase�� �ִ� Ÿ�̹�
	int bufferProtocol; // �̰Ŵ� ���� �� ���� ���ؾ��ϴ��� �����س��°ž� �� ������...

	int userIndex;	// ȸ������ + �α��� ��, �ε� ���� �����Ϳ��� �ش� �ε��� �����Ͽ� ���!
	
	int roomIndex;	// �� ����ų� ���� ��, ���� roomData�� �ε���
	bool isHost; // �ΰ��ӿ��� ȣ��Ʈ��Ȱ���� �ƴ��� üũ.

	BaseStruct* dataBuffer;	// ����� �� ģ��..�� ��� ����Ʈ������ �����ϸ鼭..�����Ⱑ �Ǿ���ȴµ�..
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

// IOCP Ŭ�������� Ż��... 
//�� ģ���� ������ ����ؼ� ���� �̱�����...?
// ������ ���� ��, ������ ���ڷ� �ѱ�°� ������..
//CUserData userData;
//CGameRoom roomData;
// �ϴ� ���� ���������, �� �����忡�� �ѱ⵵���սô�;;

class IOCPServer {
private:
	bool bIsPrintDebugMessage;

	WSADATA wsa;
	HANDLE hIOCP;
	SOCKET listenSocket;
	SOCKADDR_IN serverAddr;

	HANDLE hSaveUserDataThread;
	bool isOnSaveUserDataThread;

	//For Game
	CUserData userData;
	CGameRoom roomData;

	//std::atomic_bool isSaveOn; // ���� ���� ���� �ʿ����.. ����ȭ ���� �Ͽ� ��¥�� ��.
	bool isSaveOn;
	int selfControl;

public:
	__inline IOCPServer() : bIsPrintDebugMessage(false)
	{
		Init();
	}

	__inline IOCPServer(bool InIsPrintDebugMessage) 
		: bIsPrintDebugMessage(InIsPrintDebugMessage)
	{
		Init();
	}

	__inline ~IOCPServer()
	{
		Close();
	}
	
public:
	// Init Server
	void Init()
	{
		PrintServerInfoUI();
		LoadUserData();
		InitWinSocket();
		CreateBindListen();
	}

	void Run()
	{
		AcceptProcess();
	}

	void RunSaveUserDataThread() // ���Ŀ�, Run�� ���� �̴ϴ�.
	{
		hSaveUserDataThread = CreateThread(NULL, 0, SaveUserDate, (LPVOID)this, 0, NULL);
		//CloseHandle(hSaveUserDataThread);
	}

	void Close()
	{
		DestroyAndClean();
	}

private:
	//Init
	int GetExternalIP(char *ip);

	void PrintServerInfoUI();
	
	void LoadUserData();
	
	void InitWinSocket();

	void CreateBindListen();

	//Run
	void AcceptProcess();

	//Close
	void DestroyAndClean();

	//ThreadFunction
	static DWORD WINAPI WorkerThread(LPVOID arg);
	void WorkerThreadFunction();
	static DWORD WINAPI SaveUserDate(LPVOID arg);
	void SaveUserDataThreadFunction();
};

