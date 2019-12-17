#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h> 
#include <stdio.h> 
#include<string.h>
#include<stdlib.h>
#include<string>
#include <vector>
#include<iostream>
#include<fstream>
#include <Ws2tcpip.h> // inet_ntop, inet_pton
#pragma comment(lib,"Ws2_32.lib")

//typedef std::wstring str_t;
using namespace std;
typedef struct sendCmd 
{
	char cmd[100];
	char argv[512];

}SendCmd;

int main() {
	char myaddr[] = "127.0.0.1";
	char str[INET_ADDRSTRLEN];
	WSADATA wsaData;
	SOCKADDR_IN servAddr, clientAddr;
	SOCKET tcp_sock, hclientSock;

	SendCmd *p;
	char message[512];
	int szClntAddr; //�ּ� ����
	int str_len; // �޼��� ����

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSA error\n");
		return -1;
	} 
	// � ������ ������ ������� ���϶��̺귯�� ����
	// ���� ������ ���� �ֽ��� 2.2 ����
	// wsaData�� ������ ���������� ����� ����

	tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcp_sock == INVALID_SOCKET) {
		printf("���ϻ�������\n");
		return -1;
	}
	// ipv4 protocol, TCPŸ��, TCP ����� ���� ����

	memset(&servAddr, 0, sizeof(servAddr));
	// ���� ��Ʈ��ũ ������ ���� ����ü �ʱ�ȭ

	servAddr.sin_family = AF_INET; // ipv4 protocol
	servAddr.sin_port = htons(8081); // ��Ʈ ����
	inet_pton(AF_INET, myaddr, &(servAddr.sin_addr.s_addr));
	/*  myaddr�� ipv4������ ip�� s_addr�� ��Ʈ��ũ ����ü ��������
		��ȯ�ؼ� �־��� */

	if (bind(tcp_sock, (SOCKADDR *)&servAddr, sizeof(servAddr))) {
		printf("Bind ERROR\n");
	} 
	// bind �Լ��� ���Ͽ� ip�� ��Ʈ�� ��������. ����غ�

	if (listen(tcp_sock, 5) == SOCKET_ERROR) {
		printf("Listen ERROR\n");
	} 
	// listen���� Ŭ���̾�Ʈ ���ӿ�û�� ��ٸ����� ����
	// ������ �� ��ٸ� ����

	printf("Successful complete\n");
	printf("listen ok port %u\n", 8081);

	inet_ntop(AF_INET, &(servAddr.sin_addr), str, INET_ADDRSTRLEN);
	// str �迭�� sin_addr�� ��Ʈ��ũ �ּҸ� ��ȯ�ؼ� �־���.
	// ��Ʈ��ũ �ּ� ����ü ���� -> ĳ���� �� ���ڿ��� ��ȯ
	// ����� ���� �� �ִ� ipv4 �ּҰ� �Ǵ� ������.
	printf("%s\n", str);
	// ����� ���� �� �ֳ� ���� ���ô�.

	szClntAddr = sizeof(clientAddr);

	while (1) {
		hclientSock = accept(tcp_sock, (SOCKADDR*)&clientAddr, &szClntAddr);
		// accept�� Ŭ���̾�Ʈ�� ����ϴ� ������� ����

		if (hclientSock == INVALID_SOCKET) {
			printf("Accept ERROR\n");
			exit(1);
		}
		else
			printf("Accept Client\n");
		
		while ((str_len = recv(hclientSock, message, 512, 0)) != 0) {
			p = (SendCmd*)message;
			//printf("%s %s\n", p->cmd, p->argv);
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
				//printf("%s\n", names.back());
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
				send(hclientSock, tmp, strlen(tmp), 0); // Ŭ���̾�Ʈ���� ������ �ٽ� send
			}
		}
	}
	closesocket(tcp_sock);
	WSACleanup();
	return 0;

}