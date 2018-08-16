#include "Server.h"

static std::vector<CUserData> userData;	//�����忡���� ����ؾ��ϴµ�, ���� �����Ͱ� �̳������� ��� �Ϸ���!!, �������� �����ؼ� �� ����!����������������
static bool isSaveOn{ false };	// ���� ���� �Ǵ� (Ŭ�� ����, ���� ���� ��û ���� �� true�� ���� / ���� ����ȭ �ʿ������)

DWORD WINAPI SaveUserDate(LPVOID arg);
DWORD WINAPI WorkerThread(LPVOID arg);

int main(int argc, char * argv[])
{
#pragma region [Server UI]
	char* retIPChar;
	retIPChar = new char[20]; // IPv4�� 20 char���� Ŭ�� �׾ ����.
	GetExternalIP(retIPChar);

	printf("��������������������������\n");
	printf("�� IOCP Server  - Bridge Unity Project          \n");
	printf("��                                ver 0.1 180815\n");
	printf("��\n");
	printf("��    IP Address : %s \n", retIPChar);
	printf("��    Server Port : %d \n", SERVER_PORT);
	printf("��������������������������\n\n");

	delete[]retIPChar;

#pragma endregion

#pragma region [Load UserData]

	std::ifstream inFile("UserData.txt", std::ios::in);

	std::string ID;
	int PW, winCount, loseCount, Money;
	int userDataCount{};

	inFile >> userDataCount;
	userData.reserve(userDataCount);

	for (int i = 0; i < userDataCount; i++) {
		inFile >> ID >> PW >> winCount >> loseCount >> Money;

		userData.emplace_back(ID, PW, winCount, loseCount, Money);
	}

	inFile.close();

	std::cout << "  - UserData Load Complete! " << std::endl << std::endl;

	//for (auto i : userData) 
	//{
	//	std::cout << i.GetID() << " " << i.GetPW() << " " << i.GetWinCount() << " " << i.GetLoseCount() << "  " << i.GetMoney() << std::endl;
	//}

#pragma endregion

#pragma region [ ���� �ʱ�ȭ �� ����� �Ϸ� ��Ʈ ���� ]
	//���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	// ����� �Ϸ� ��Ʈ ����
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	/*
		CreateIoCompletionPort�� �ΰ��� ������ ��!
			1. ����� �Ϸ� ��Ʈ ����
			2. ���ϰ� ����� �Ϸ� ��Ʈ ���� (IO��ġ�� IOCP����)

		1��° ���ڰ�,  IOCP�� ������ �ڵ�, �����ô� INVALID_HANDLE_VALUE�� ���ڷ� �ѱ�
		2��° ���ڰ�,  IOCP �ڵ�, ù �����ô� NULL
		3��° ���ڰ�, IO�Ϸ�� �Ѿ ��, ����ڰ� �ѱ�� ���� �� �ѱ�
		4��° ���ڰ�, �ѹ��� ������ �ִ� ������ ����, 0 �ѱ�� ���μ��� ���ڷ� �ڵ� ������
	*/

	if (hcp == NULL)
	{
		printf("����� �Ϸ� ��Ʈ ����");
		return 1;
	}

	// CPU ���� Ȯ��
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// CPU ���� == 12�� * 2���� �۾��� ������ ����
	// IO�۾��� �Ϸ�� ��, �Ϸ�� IO�� ���� ó���� ������ ������ Ǯ�� �����Ѵ�.
	// �Ϲ������� ������ Ǯ�� ũ��� ���μ��� ������ 2�� ������ �Ҵ��Ѵ�.
	HANDLE hThread;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; ++i)
	{
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (hThread == NULL) return 1;
		CloseHandle(hThread);
	}

#pragma endregion

#pragma region [ ���� ���� ��, ���ε�, ���� ]

	//Socket()
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) err_quit((char *)"socket()");

	//bind()
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVER_PORT);
	int retVal = bind(listenSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (retVal == SOCKET_ERROR) err_quit((char *)"bind()");

	// Listen()!
	retVal = listen(listenSocket, SOMAXCONN);
	if (retVal == SOCKET_ERROR) err_quit((char *)"listen()");

#pragma endregion

#pragma region [ Thread Run! Accept and Save UserData ]

	printf("  - Dedicated server activated!\n\n");

	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	int addrLength;
	DWORD recvBytes, flags;

	HANDLE hSaveUserDataThread;
	hSaveUserDataThread = CreateThread(NULL, 0, SaveUserDate, NULL, 0, NULL);
	CloseHandle(hSaveUserDataThread);

	while (7) {
		//accept()
		addrLength = sizeof(clientAddr);
		clientSocket = accept(listenSocket, (SOCKADDR *)&clientAddr, &addrLength);
		if (clientSocket == INVALID_SOCKET)
		{
			err_display((char *)"accept()");
			break;
		}

		// Ŭ���̾�Ʈ ������ ����(Accept) ���� �˸�
		printf("[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� =%s, Port ��ȣ = %d \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		// ���ϰ� ����� �Ϸ� ��Ʈ ����
		CreateIoCompletionPort((HANDLE)clientSocket, hcp, clientSocket, 0);

		// ���� ���� ����ü �Ҵ�
		SOCKETINFO *ptr = new SOCKETINFO;
		if (ptr == NULL) break;

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

		ptr->sock = clientSocket;
		ptr->isRecvTrue = true;
		ptr->bufferProtocol = 0;

		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUF_SIZE;

		// �񵿱� ������� ����
		flags = 0;
		retVal = WSARecv(
			clientSocket, // Ŭ���̾�Ʈ ����
			&ptr->wsabuf, // ���� ������ ������ ������
			1,			 // ������ �Է� ������ ����
			&recvBytes,  // recv ��� ���� ����Ʈ ��, IOCP������ �񵿱� ������� ������� �����Ƿ� nullPtr�� �Ѱܵ� ����
			&flags,		 // recv�� ���� �÷���
			&ptr->overlapped, // overlapped����ü�� ������
			NULL			// IOCP������ ������� �����Ƿ� NULL, nullptr�Ѱܵ� ����
		);

		if (retVal == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				err_display((char *)"WSARecv()");
			}

			continue;
		}
	}
#pragma endregion

#pragma region [ Destroy() && plzDoNotQuit! ]
	char plzDoNotQuit{};
	std::cin >> plzDoNotQuit;

	closesocket(listenSocket);
	WSACleanup();

	return 0;
#pragma endregion
}

DWORD WINAPI SaveUserDate(LPVOID arg) {
	while (7) {
		Sleep(10000);

		if (isSaveOn) {
			isSaveOn = false;

			Sleep(2000);
			std::ofstream outFile("UserData.txt", std::ios::out);

			outFile << userData.size() << std::endl;

			for (auto i : userData) {
				outFile << " " << i.GetID()
					<< " " << i.GetPW()
					<< " " << i.GetWinCount()
					<< " " << i.GetLoseCount()
					<< " " << i.GetMoney()
					<< std::endl;
			}
			outFile.close();

			std::cout << "[ System(Server Core) - UserDataSave ]" << std::endl;
			isSaveOn = false;
			Sleep(2000);
		}
	}
	return 0;
}

DWORD WINAPI WorkerThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;

	int retVal{};
	int recvType{};
	int sendType{};

	while (7)
	{

#pragma region [ Wait For Thread ]
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
#pragma endregion

#pragma region [ Get Socket and error Exception ]
		std::cout << "newThread Fire!" << std::endl;

		// �Ҵ���� ���� ��! Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientAddr;
		int addrLength = sizeof(clientAddr);
		getpeername(ptr->sock, (SOCKADDR *)&clientAddr, &addrLength);

		//�񵿱� ����� ��� Ȯ�� // �ƹ��͵� �Ⱥ��� ����, �ش� Ŭ���̾�Ʈ ���ӿ� ������ ��������� �Ǵ�, �ݾƹ����ڴ�!
		// �ٵ� �̰� ���ڼ����ϋ��� �׷��ߵǴ°� �ƴϾ�???? ���� �Ӹ� ���� �𸦼��� ����
		if (retVal == 0 || cbTransferred == 0)
		{
			std::cout << "DEBUG - Error or Exit Client A" << std::endl;

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

#pragma endregion

		if (ptr->isRecvTrue)
		{
			if (ptr->bufferProtocol == 0) {
				recvType = (int&)(ptr->buf);

				if (recvType == DEMAND_LOGIN) {
					ptr->bufferProtocol = DEMAND_LOGIN;
					ptr->isRecvTrue = true;

					std::cout << "�α��� �õ��� �޾ҽ��ϴ�."<< std::endl;

					//������ �ޱ�
					ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
					ptr->wsabuf.buf = ptr->buf;
					ptr->wsabuf.len = BUF_SIZE;

					DWORD recvBytes;
					DWORD flags{ 0 };
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
					else {
						std::cout << "��ǵ� �α��� �ٷ� �޾ƹ��ȴ�...¥�� ���� ƽ���� ó�����ٰ�?? " << std::endl;
					}
				}
			}
			else if (ptr->bufferProtocol == DEMAND_LOGIN) {

				DemandLoginStruct demandLogin = (DemandLoginStruct&)(ptr->buf);
				std::cout << "���̵� ��й�ȣ�� �Է� �޾ҽ��ϴ�. ID:  " << demandLogin.ID << "  PW : " << demandLogin.PW << "  type : " << demandLogin.type << std::endl;
				// 1�϶� �α��� ���� ���̵�, 2�϶� �α��� �߸��� ��й�ȣ, 3�϶� �̹� �α����� ���̵�, 4�϶� ȸ������ �ߺ��� ���̵�! protocol Sync plz!  // 5�϶��� ����??
				
				int failReason = 0;

				if (demandLogin.type == 1) {
					int winRate{};
					int loseRate{};
					int money{};

					for (auto &i : userData)
					{
						if ( i.GetID().compare(demandLogin.ID))
						{
							if (!i.GetIsLogin())
							{
								if (i.GetPW() == demandLogin.PW)
								{
									i.SetIsLogin(true);
									winRate = i.GetWinCount();
									loseRate = i.GetLoseCount();
									money = i.GetMoney();
								}
								else
								{
									failReason = 2;
									break;
								}
							}
							else
							{
								failReason = 3;
								break;
							}
						}
					}

					if (!failReason) {

						std::cout << "�α��ο� �����߽��ϴ�. " << std::endl;

						ptr->dataBuffer = new PermitLoginStruct(winRate, loseRate, money);

						ptr->bufferProtocol = PERMIT_LOGIN;
						ptr->isRecvTrue = false;
						//permitLoginStruct* a = static_cast<PermitLoginStruct *>(ptr->dataBuffer);

						// ������ ������
						ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

						int buffer = PERMIT_LOGIN;
						memcpy(ptr->buf, (char*)&buffer, sizeof(int));
						ptr->dataSize = sizeof(int);

						ptr->wsabuf.buf = ptr->buf; // ptr->buf;
						ptr->wsabuf.len = ptr->dataSize;

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
					else if (failReason){

						std::cout << "�α��ο� �����߽��ϴ�.  �ش� ������ : " << failReason << std::endl;

						ptr->dataBuffer = new FailLoginStruct(failReason);
						ptr->bufferProtocol = FAIL_LOGIN;
						ptr->isRecvTrue = false;

						// ������ ������
						ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

						int buffer = FAIL_LOGIN;
						memcpy(ptr->buf, (char*)&buffer, sizeof(buffer));
						ptr->dataSize = sizeof(buffer);

						ptr->wsabuf.buf = ptr->buf; // ptr->buf;
						ptr->wsabuf.len = ptr->dataSize;

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
				}
				else if (demandLogin.type == 2) {
					for (auto &i : userData)
					{
						if (i.GetID().compare(demandLogin.ID)) {
							failReason = 4;
							break;
						}
					}

					if (!failReason) {

						std::cout << "ȸ������ �� �α��ο� �����߽��ϴ�. " << std::endl;

						userData.emplace_back(demandLogin.ID, demandLogin.PW);

						ptr->dataBuffer = new PermitLoginStruct(0, 0, 0);
						ptr->bufferProtocol = PERMIT_LOGIN;
						ptr->isRecvTrue = false;

						// ������ ������
						ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

						int buffer = PERMIT_LOGIN;
						memcpy(ptr->buf, (char*)&buffer, sizeof(buffer));
						ptr->dataSize = sizeof(buffer);

						ptr->wsabuf.buf = ptr->buf; // ptr->buf;
						ptr->wsabuf.len = ptr->dataSize;

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
					else if (failReason) {

						std::cout << "ȸ�����Կ� �����߽��ϴ�.  �ش� ������ : " << failReason << std::endl;

						ptr->dataBuffer = new FailLoginStruct(failReason);
						ptr->bufferProtocol = FAIL_LOGIN;
						ptr->isRecvTrue = false;

						// ������ ������
						ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

						int buffer = FAIL_LOGIN;
						memcpy(ptr->buf, (char*)&buffer, sizeof(buffer));
						ptr->dataSize = sizeof(buffer);

						ptr->wsabuf.buf = ptr->buf; // ptr->buf;
						ptr->wsabuf.len = ptr->dataSize;

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
				}
			}
		}
		else if (!(ptr->isRecvTrue))
		{
			if (ptr->bufferProtocol == PERMIT_LOGIN)
			{
				ptr->bufferProtocol = 0;
				ptr->isRecvTrue = 0;

				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				memcpy(ptr->buf, (char*)&(ptr->dataBuffer), sizeof(PermitLoginStruct));
				ptr->dataSize = sizeof(PermitLoginStruct);

				delete (ptr->dataBuffer);

				ptr->wsabuf.buf = ptr->buf; // ptr->buf;
				ptr->wsabuf.len = ptr->dataSize;

				DWORD sendBytes;
				retVal = WSASend(ptr->sock,	&ptr->wsabuf, 1, &sendBytes, 0, &ptr->overlapped, NULL);
				
				if (retVal == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						err_display((char *)"WSASend()");
					}
					continue;
				}
			}
			else if (ptr->bufferProtocol == FAIL_LOGIN)
			{
				ptr->bufferProtocol = 0;
				ptr->isRecvTrue = 0;

				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				memcpy(ptr->buf, (char*)&(ptr->dataBuffer), sizeof(FailLoginStruct));
				ptr->dataSize = sizeof(FailLoginStruct);

				delete (ptr->dataBuffer);

				ptr->wsabuf.buf = ptr->buf; // ptr->buf;
				ptr->wsabuf.len = ptr->dataSize;

				DWORD sendBytes;
				retVal = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendBytes, 0, &ptr->overlapped, NULL);

				if (retVal == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						err_display((char *)"WSASend()");
					}
					continue;
				}
			}


		}

		// ������ ���۷� ����
		/*
		if (ptr->recvBytes == 0)
		{
		std::cout << "DEBUG - B" << std::endl;

		//cbTransferred�� ���� �������� ũ�⸦ ����!! --> �� ����(����, ����, ����)�� 1byte, (�ѱ�)2byte
		ptr->recvBytes = cbTransferred;
		ptr->sendBytes = 0;


		// ���� ������ ���
		ptr->buf[ptr->recvBytes] = '\0';

		//printf(" [TCP %s :%d] %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), ptr->buf);

		std::cout << "[Debug] : ���۵� Size : " << cbTransferred << "  ���� :  " << (int&)(ptr->buf) << std::endl;

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
		*/
	}
};
