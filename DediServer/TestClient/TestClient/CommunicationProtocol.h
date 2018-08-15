#pragma once

#include "TestClient.h"

enum Protocol {
	DEMAND_LOGIN = 100
	, FAIL_LOGIN = 101
	, PERMIT_LOGIN = 102
};

// type 100�϶�, ������ �ٷ� ���� �����ִ� ����ü
struct DemandLoginStruct {
	int type{};	// 1�϶��� �α���, 2�϶��� ȸ������
	int PW{};
	std::string ID;
};

// type 101 Server -> Client �α��� ����, ȸ������ ����
struct FailLoginStruct {
};

// type 102 Server -> Client �α��� ����, Lobby����, �������� ����
struct PermitLoginStruct {
	int winCount{};
	int loseCount{};
	int money{};
};