#include "TestClient.h"
#include "CommunicationProtocol.h"


struct TestStruct {
	int Type;
	DemandLoginStruct demand;
};

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit((char *)"socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit((char *)"connect()");

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE + 1];
	int len;

	int recvType{};
	int sendType{};

	// ������ ������ ���
	while (7) {
		// ������ �Է�
		printf("\n[�ȳ�!] ���� �������� �ѹ��� �Է��ϼ���  0 = ���� , 100 = �α��� �� ȸ������ :  ");
		rewind(stdin);
		std::cin >> sendType;

		if (!sendType)
			break;
		// '\n' ���� ����
		//len = strlen(buf);
		//if (buf[len - 1] == '\n')
		//	buf[len - 1] = '\0';
		//if (strlen(buf) == 0)
		//	break;
		if (sendType == DEMAND_LOGIN) {
			DemandLoginStruct demandLogin;
			std::cout << "ID�Է� ��, PW �Է��ϸ�, �� ��, Type ( 1 �α��� 2 ȸ������) �Է��մϴ�." << std::endl;
			std::cin >> demandLogin.ID;
			std::cin >> demandLogin.PW;
			std::cin >> demandLogin.type;

			TestStruct testStruct;
			testStruct.Type = sendType;
			testStruct.demand = demandLogin;

			retval = send(sock, (char*)&testStruct, sizeof(testStruct), 0);
			printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);

			if (retval == SOCKET_ERROR)
			{
				err_display((char *)"send()");
				break;
			}
		}
		/*
						// ������ �ޱ�
			retval = recvn(sock, (char*)&recvType, sizeof(int), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display((char *)"recv()");
				break;
			}
			else if (retval == 0) {

				break;
			}

			if (recvType == PERMIT_LOGIN)
			{
				PermitLoginStruct permitLogin;

				retval = recvn(sock, (char*)&permitLogin, sizeof(PermitLoginStruct), 0);

				std::cout << "�α��� or ȸ������ ����!  WinCount : " << permitLogin.winCount << "LoseCount : " << permitLogin.loseCount << " Money : " << permitLogin.money << std::endl;
			}
			else if (recvType == FAIL_LOGIN)
			{
				FailLoginStruct failLogin;

				retval = recvn(sock, (char*)&failLogin, sizeof(FailLoginStruct), 0);

				std::cout << "�α��� or ȸ������ ����!" << std::endl;
				std::cout << "������ : " << failLogin.type << std::endl;

			}
		}
		*/
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}