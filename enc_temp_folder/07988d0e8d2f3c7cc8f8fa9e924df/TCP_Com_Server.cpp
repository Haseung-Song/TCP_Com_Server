// TCP_Com_Server.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <Winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

// [클라이언트 UI] IP, Port와 맞춤
#define PORT 2000
#define BUFSIZE 512

int main()
{
	int retval;

	//윈도우 소켓(원속) 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return -1;

	// 소켓 확인
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return -1;

	// 연결 확인(listen)
	SOCKADDR_IN servAddr; // IPv4 주소 정보를 담는 구조체 (IP + Port 저장하는 통)
	ZeroMemory(&servAddr, sizeof(servAddr)); // ZeroMemory(주소, 크기); 사실상 초기화 부분 (구조체를 0으로 초기화해서 안전하게 함.)
	servAddr.sin_family = AF_INET; // IPv4 주소 체계 (참고: IPv6: AF_INET6)
	servAddr.sin_port = htons(PORT); // htons 쓰는 이유: 컴퓨터는 보통 Little Endian, 네트워크는 Big Endian, 그래서 변환이 필요함.
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // 모든 IP에서 들어오는 요청을 받겠다는 의미.
	// 서버를 특정 포트에 바인딩해서 외부 접속 받을 준비를 함.
	if (bind(listenSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) // bind(소켓, 주소정보, 크기) 메서드
	{
		closesocket(listenSocket);
		return -1;
	}
	// bind() 단계까지 끝난 소켓이므로, 이제 "서버 역할"할 준비된 상태
	retval = listen(listenSocket, SOMAXCONN); // 이 소켓을 클라이언트 연결 대기 상태로 (SOMAXCONN: 동시에 대기 가능한 최대 연결 요청 수)
	if (retval == SOCKET_ERROR)
	{
		printf("listen() Error\n");
		closesocket(listenSocket);
		return -1;
	}

	SOCKET clientSocket; // 클라이언트랑 통신할 새로운 소켓 (실제 통신용)
	SOCKADDR_IN clientAddr; // 접속한 클라이언트의 IP / Port 정보 담는 구조체
	int length; // 구조체 크기 전달용
	char buf[BUFSIZE + 1]; // 데이터 받을 버퍼
	// 서버는 클라이언트 통신을 계속 기다려야 하니까 종료 없이 반복
	while (1)
	{
		// 접속
		length = sizeof(clientAddr);
		// 대기 중이던 클라이언트 연결을 하나 받아서 새로운 소켓 생성
		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &length);
		// 실패 시, 에러 출력 후, 재루프 돌아서 재접속 기다림.
		if (clientSocket == INVALID_SOCKET)
		{
			printf("accept() Error\n");
			continue;
		}

		char ipStr[INET_ADDRSTRLEN];
		// IP 변환을 따로 한 후에 ipstr 변수에 저장 함.
		inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
		// 클라이언트의 정보 출력
		printf("\n클라이언트 IP: %s, Port: %d\n",
			ipStr,
			ntohs(clientAddr.sin_port));

		// 클라이언트와 계속 통신
		while (1)
		{
			// 클라이언트가 보낸 데이터를 buf로 수신한다.
			retval = recv(clientSocket, buf, BUFSIZE, 0);
			// 수신 실패 시, 루프 종료!
			if (retval == SOCKET_ERROR)
			{
				printf("recv() Error");
				break;
			}
			// 그게 아니면, 클라이언트가 정상적으로 연결을 끊음.
			else if (retval == 0)
				break;
			buf[retval] = '\0'; // 문자열 끝 표시

			char ipStr[INET_ADDRSTRLEN];
			// IP 변환을 따로 한 후에 ipstr 변수에 저장 함.
			inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
			// [클라이언트 UI]가 보낸 데이터 HEX DATA 출력.
			printf("[TCP %s : %d] RECV HEX : ",
				ipStr,
				ntohs(clientAddr.sin_port));

			// 수신한 데이터(Byte)를 16진수(HEX) 형태로 출력
			for (int i = 0; i < retval; i++)
			{
				// %02X
				// 2자리 HEX 출력
				// 빈 자리는 0으로 채움
				// unsigned char 변환으로 음수 출력 방지
				printf("%02X ", (unsigned char)buf[i]);
			}

			// 출력 줄바꿈
			printf("\n");

			// 받은 데이터 그대로 클라이언트에게 재송신!
			retval = send(clientSocket, buf, retval, 0);
			// 전송 실패 시, 루프 종료!
			if (retval == SOCKET_ERROR)
			{
				printf("send() Error");
				break;
			}

		}
		// 클라이언트와 연결 종료!
		closesocket(clientSocket);
	}
	WSACleanup(); // Winsock 전체 종료 (서버 종료 시, 1번만 수행)
	return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴
// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
