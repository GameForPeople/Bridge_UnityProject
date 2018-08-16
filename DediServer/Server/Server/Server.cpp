#include "Server.h"

static std::vector<CUserData> userData;	//床傾球拭辞亀 紫遂背醤馬澗汽, 政煽 汽戚斗亜 位蟹弦生檎 嬢胸惟 馬形壱!!, 穿蝕生稽 識情背辞 袷 床切!ぞぞぞぞぞぞぞぞ
static bool isSaveOn{ false };	// 煽舌 食採 毒舘 (適虞拭 税背, 舛左 痕井 推短 閤聖 獣 true稽 痕井 / 瓜戚 疑奄鉢 琶推蒸聖牛)

DWORD WINAPI SaveUserDate(LPVOID arg);
DWORD WINAPI WorkerThread(LPVOID arg);

int main(int argc, char * argv[])
{
#pragma region [Server UI]
	char* retIPChar;
	retIPChar = new char[20]; // IPv4亜 20 char左陥 適析 宋嬢亀 蒸製.
	GetExternalIP(retIPChar);

	printf("＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝\n");
	printf("＝ IOCP Server  - Bridge Unity Project          \n");
	printf("＝                                ver 0.1 180815\n");
	printf("＝\n");
	printf("＝    IP Address : %s \n", retIPChar);
	printf("＝    Server Port : %d \n", SERVER_PORT);
	printf("＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝\n\n");

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

#pragma region [ 制紗 段奄鉢 貢 脊窒径 刃戟 匂闘 持失 ]
	//制紗 段奄鉢
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	// 脊窒径 刃戟 匂闘 持失
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	/*
		CreateIoCompletionPort澗 砧亜走 蝕拝聖 敗!
			1. 脊窒径 刃戟 匂闘 持失
			2. 社掴引 脊窒径 刃戟 匂闘 尻衣 (IO舌帖人 IOCP尻衣)

		1腰属 昔切葵,  IOCP人 尻衣拝 輩級, 持失獣澗 INVALID_HANDLE_VALUE研 昔切稽 角沿
		2腰属 昔切葵,  IOCP 輩級, 湛 持失獣澗 NULL
		3腰属 昔切葵, IO刃戟獣 角嬢哀 葵, 紫遂切亜 角奄壱 粛精 葵 角沿
		4腰属 昔切葵, 廃腰拭 疑拙拝 置企 什傾球 鯵呪, 0 角奄檎 覗稽室什 収切稽 切疑 走舛喫
	*/

	if (hcp == NULL)
	{
		printf("脊窒径 刃戟 匂闘 持失");
		return 1;
	}

	// CPU 鯵呪 溌昔
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// CPU 鯵呪 == 12鯵 * 2鯵税 拙穣切 什傾球 持失
	// IO拙穣戚 刃戟吉 板, 刃戟吉 IO拭 企廃 坦軒研 呪楳拝 什傾球 熱聖 姥失廃陥.
	// 析鋼旋生稽 什傾球 熱税 滴奄澗 覗稽室辞 鯵呪税 2壕 舛亀研 拝雁廃陥.
	HANDLE hThread;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; ++i)
	{
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (hThread == NULL) return 1;
		CloseHandle(hThread);
	}

#pragma endregion

#pragma region [ 社掴 持失 貢, 郊昔球, 軒充 ]

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

		// 適虞戚情闘 辞獄拭 羨紗(Accept) 敗聖 硝顕
		printf("[TCP 辞獄] 適虞戚情闘 羨紗 : IP 爽社 =%s, Port 腰硲 = %d \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		// 社掴引 脊窒径 刃戟 匂闘 尻衣
		CreateIoCompletionPort((HANDLE)clientSocket, hcp, clientSocket, 0);

		// 社掴 舛左 姥繕端 拝雁
		SOCKETINFO *ptr = new SOCKETINFO;
		if (ptr == NULL) break;

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

		ptr->sock = clientSocket;
		ptr->isRecvTrue = true;
		ptr->bufferProtocol = 0;

		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUF_SIZE;

		// 搾疑奄 脊窒径税 獣拙
		flags = 0;
		retVal = WSARecv(
			clientSocket, // 適虞戚情闘 社掴
			&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
			1,			 // 汽戚斗 脊径 獄遁税 鯵呪
			&recvBytes,  // recv 衣引 石精 郊戚闘 呪, IOCP拭辞澗 搾疑奄 号縦生稽 紫遂馬走 省生糠稽 nullPtr研 角移亀 巷号
			&flags,		 // recv拭 紫遂吃 巴掘益
			&ptr->overlapped, // overlapped姥繕端税 匂昔斗
			NULL			// IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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
		std::cout << "i'm wait Thread" << std::endl;

#pragma region [ Wait For Thread ]
		//搾疑奄 脊窒径 奄陥軒奄
		DWORD cbTransferred;
		SOCKET clientSocket;
		SOCKETINFO *ptr;

		// 脊窒径 刃戟 匂闘拭 煽舌吉 衣引研 坦軒馬奄 是廃 敗呪 // 企奄 雌殿亜 喫
		retVal = GetQueuedCompletionStatus(
			hcp, //脊窒径 刃戟 匂闘 輩級
			&cbTransferred, //搾疑奄 脊窒径 拙穣生稽, 穿勺吉 郊戚闘 呪亜 食奄拭 煽舌吉陥.
			(LPDWORD)&clientSocket, //敗呪 硲窒 獣 穿含廃 室腰属 昔切(32搾闘) 亜 食奄拭 煽舌吉陥.
			(LPOVERLAPPED *)&ptr, //Overlapped 姥繕端税 爽社葵
			INFINITE // 企奄 獣娃 -> 凹随�� 猿走 巷廃企
		);
#pragma endregion

#pragma region [ Get Socket and error Exception ]
		std::cout << "new Thread Fire!!" << std::endl;

		// 拝雁閤精 社掴 聡! 適虞戚情闘 舛左 条奄
		SOCKADDR_IN clientAddr;
		int addrLength = sizeof(clientAddr);
		getpeername(ptr->sock, (SOCKADDR *)&clientAddr, &addrLength);

		//搾疑奄 脊窒径 衣引 溌昔 // 焼巷依亀 照左馨 凶澗, 背雁 適虞戚情闘 羨紗拭 庚薦亜 持延依生稽 毒舘, 丸焼獄軒畏陥!
		// 悦汽 戚暗 拭坪辞獄析��幻 益掘醤鞠澗暗 焼艦醤???? 侯坐 績原 校坐 乞研呪亀 赤走
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

			printf("[TCP 辞獄] 適虞戚情闘 曽戟 : IP 爽社 =%s, 匂闘 腰硲 =%d\n",
				inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			delete ptr;
			continue;
		}

#pragma endregion

		if (ptr->bufferProtocol == -1)
		{
			ptr->bufferProtocol = 0;
			std::cout << "ptr->bufferProtocol == -1" << std::endl;

			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUF_SIZE;

			// 搾疑奄 脊窒径税 獣拙
			DWORD flags = 0;
			DWORD recvBytes{};

			retVal = WSARecv(
				clientSocket, // 適虞戚情闘 社掴
				&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
				1,			 // 汽戚斗 脊径 獄遁税 鯵呪
				&recvBytes,  // recv 衣引 石精 郊戚闘 呪, IOCP拭辞澗 搾疑奄 号縦生稽 紫遂馬走 省生糠稽 nullPtr研 角移亀 巷号
				&flags,		 // recv拭 紫遂吃 巴掘益
				&ptr->overlapped, // overlapped姥繕端税 匂昔斗
				NULL			// IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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
		else if (ptr->isRecvTrue)
		{
			if (ptr->bufferProtocol == 0) {
				recvType = (int&)(ptr->buf);
				std::cout << "ptr->bufferProtocol == 0 , recvType == "<< recvType << std::endl;

				if (recvType == DEMAND_LOGIN) {
					ptr->bufferProtocol = DEMAND_LOGIN;
					ptr->isRecvTrue = true;

					std::cout << "稽益昔 獣亀研 閤紹柔艦陥."<< std::endl;

					//汽戚斗 閤奄
					ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
					ptr->wsabuf.buf = ptr->buf;
					ptr->wsabuf.len = BUF_SIZE;

					DWORD recvBytes;
					DWORD flags{ 0 };
					retVal = WSARecv(
						ptr->sock, // 適虞戚情闘 社掴
						&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
						1, // 汽戚斗 脊径 獄遁税 鯵呪
						&recvBytes, // recv 衣引 石精 郊戚闘 呪, IOCP拭辞澗 搾疑奄 号縦生稽 紫遂馬走 省生糠稽 nullPtr研 角移亀 巷号
						&flags,  // recv拭 紫遂吃 巴掘益
						&ptr->overlapped, // overlapped姥繕端税 匂昔斗
						NULL // IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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
						std::cout << "巨固球 稽益昔 郊稽 閤焼獄携陥...促杷 陥製 峠拭辞 坦軒背匝杏?? " << std::endl;
					}
				}
			}
			else if (ptr->bufferProtocol == DEMAND_LOGIN) {

				DemandLoginStruct demandLogin = (DemandLoginStruct&)(ptr->buf);
				std::cout << "焼戚巨 搾腔腰硲研 脊径 閤紹柔艦陥. ID:  " << demandLogin.ID << "  PW : " << demandLogin.PW << "  type : " << demandLogin.type << std::endl;
				// 1析凶 稽益昔 蒸澗 焼戚巨, 2析凶 稽益昔 設公吉 搾腔腰硲, 3析凶 戚耕 稽益昔廃 焼戚巨, 4析凶 噺据亜脊 掻差吉 焼戚巨! protocol Sync plz!  // 5析凶澗 舛雌??
				
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

						std::cout << "稽益昔拭 失因梅柔艦陥. " << std::endl;

						ptr->dataBuffer = new PermitLoginStruct(winRate, loseRate, money);

						ptr->bufferProtocol = PERMIT_LOGIN;
						ptr->isRecvTrue = false;
						//permitLoginStruct* a = static_cast<PermitLoginStruct *>(ptr->dataBuffer);

						// 汽戚斗 左鎧奄
						ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

						int buffer = PERMIT_LOGIN;
						memcpy(ptr->buf, (char*)&buffer, sizeof(int));
						ptr->dataSize = sizeof(int);

						ptr->wsabuf.buf = ptr->buf; // ptr->buf;
						ptr->wsabuf.len = ptr->dataSize;

						DWORD sendBytes;

						retVal = WSASend(
							ptr->sock, // 適虞戚情闘 社掴
							&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
							1, // 汽戚斗 脊径 獄遁税 鯵呪
							&sendBytes, // Send 郊戚闘 呪...?
							0, // ??????????????
							&ptr->overlapped, // overlapped姥繕端税 匂昔斗
							NULL // IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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

						std::cout << "稽益昔拭 叔鳶梅柔艦陥.  背雁 紫政澗 : " << failReason << std::endl;

						ptr->dataBuffer = new FailLoginStruct(failReason);
						ptr->bufferProtocol = FAIL_LOGIN;
						ptr->isRecvTrue = false;

						// 汽戚斗 左鎧奄
						ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

						int buffer = FAIL_LOGIN;
						memcpy(ptr->buf, (char*)&buffer, sizeof(buffer));
						ptr->dataSize = sizeof(buffer);

						ptr->wsabuf.buf = ptr->buf; // ptr->buf;
						ptr->wsabuf.len = ptr->dataSize;

						DWORD sendBytes;
						retVal = WSASend(
							ptr->sock, // 適虞戚情闘 社掴
							&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
							1, // 汽戚斗 脊径 獄遁税 鯵呪
							&sendBytes, // Send 郊戚闘 呪...?
							0, // ??????????????
							&ptr->overlapped, // overlapped姥繕端税 匂昔斗
							NULL // IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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

						std::cout << "噺据亜脊 貢 稽益昔拭 失因梅柔艦陥. " << std::endl;

						userData.emplace_back(demandLogin.ID, demandLogin.PW);

						ptr->dataBuffer = new PermitLoginStruct(0, 0, 0);
						ptr->bufferProtocol = PERMIT_LOGIN;
						ptr->isRecvTrue = false;

						// 汽戚斗 左鎧奄
						ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

						int buffer = PERMIT_LOGIN;
						memcpy(ptr->buf, (char*)&buffer, sizeof(buffer));
						ptr->dataSize = sizeof(buffer);

						ptr->wsabuf.buf = ptr->buf; // ptr->buf;
						ptr->wsabuf.len = ptr->dataSize;

						DWORD sendBytes;
						retVal = WSASend(
							ptr->sock, // 適虞戚情闘 社掴
							&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
							1, // 汽戚斗 脊径 獄遁税 鯵呪
							&sendBytes, // Send 郊戚闘 呪...?
							0, // ??????????????
							&ptr->overlapped, // overlapped姥繕端税 匂昔斗
							NULL // IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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

						std::cout << "噺据亜脊拭 叔鳶梅柔艦陥.  背雁 紫政澗 : " << failReason << std::endl;

						ptr->dataBuffer = new FailLoginStruct(failReason);
						ptr->bufferProtocol = FAIL_LOGIN;
						ptr->isRecvTrue = false;

						// 汽戚斗 左鎧奄
						ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));

						int buffer = FAIL_LOGIN;
						memcpy(ptr->buf, (char*)&buffer, sizeof(buffer));
						ptr->dataSize = sizeof(buffer);

						ptr->wsabuf.buf = ptr->buf; // ptr->buf;
						ptr->wsabuf.len = ptr->dataSize;

						DWORD sendBytes;
						retVal = WSASend(
							ptr->sock, // 適虞戚情闘 社掴
							&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
							1, // 汽戚斗 脊径 獄遁税 鯵呪
							&sendBytes, // Send 郊戚闘 呪...?
							0, // ??????????????
							&ptr->overlapped, // overlapped姥繕端税 匂昔斗
							NULL // IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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
				std::cout << "ptr->bufferProtocol == PERMIT_LOGIN" << std::endl;
				ptr->bufferProtocol = -1;
				ptr->isRecvTrue = true;

				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				memcpy(ptr->buf, (char*)(ptr->dataBuffer), sizeof(PermitLoginStruct));
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
				std::cout << "ptr->bufferProtocol == FAIL_LOGIN" << std::endl;
				ptr->bufferProtocol = -1;
				ptr->isRecvTrue = true;

				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				memcpy(ptr->buf, (char*)(ptr->dataBuffer), sizeof(FailLoginStruct));
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
		// 汽戚斗 穿勺勲 飴重
		/*
		if (ptr->recvBytes == 0)
		{
		std::cout << "DEBUG - B" << std::endl;

		//cbTransferred澗 閤精 汽戚斗税 滴奄研 倶敗!! --> 廃 越切(慎嬢, 収切, 因拷)拭 1byte, (廃越)2byte
		ptr->recvBytes = cbTransferred;
		ptr->sendBytes = 0;


		// 閤精 汽戚斗 窒径
		ptr->buf[ptr->recvBytes] = '\0';

		//printf(" [TCP %s :%d] %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), ptr->buf);

		std::cout << "[Debug] : 穿勺吉 Size : " << cbTransferred << "  鎧遂 :  " << (int&)(ptr->buf) << std::endl;

		}
		else
		{
		std::cout << "DEBUG - C" << std::endl;

		ptr->sendBytes += cbTransferred;

		}

		if (ptr->recvBytes > ptr->sendBytes)
		{
		std::cout << "DEBUG - D" << std::endl;

		// 汽戚斗 左鎧奄
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->wsabuf.buf = ptr->buf + ptr->sendBytes;
		ptr->wsabuf.len = ptr->recvBytes - ptr->sendBytes;

		DWORD sendBytes;
		retVal = WSASend(
		ptr->sock, // 適虞戚情闘 社掴
		&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
		1, // 汽戚斗 脊径 獄遁税 鯵呪
		&sendBytes, // Send 郊戚闘 呪...?
		0, // ??????????????
		&ptr->overlapped, // overlapped姥繕端税 匂昔斗
		NULL // IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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

		//汽戚斗 閤奄
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUF_SIZE;

		DWORD recvBytes;
		DWORD flags{0};
		retVal = WSARecv(
		ptr->sock, // 適虞戚情闘 社掴
		&ptr->wsabuf, // 石聖 汽戚斗 獄遁税 匂昔斗
		1, // 汽戚斗 脊径 獄遁税 鯵呪
		&recvBytes, // recv 衣引 石精 郊戚闘 呪, IOCP拭辞澗 搾疑奄 号縦生稽 紫遂馬走 省生糠稽 nullPtr研 角移亀 巷号
		&flags,  // recv拭 紫遂吃 巴掘益
		&ptr->overlapped, // overlapped姥繕端税 匂昔斗
		NULL // IOCP拭辞澗 紫遂馬走 省生糠稽 NULL, nullptr角移亀 巷号
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
