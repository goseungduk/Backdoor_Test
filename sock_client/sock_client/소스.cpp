#include<winsock2.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<iostream>
#include <Ws2tcpip.h>
#pragma comment (lib,"ws2_32.lib")

using namespace std;

typedef struct sendCmd // struct of sending data
{
	char cmd[100];
	char argv[512];
}SendCmd;

constexpr unsigned int Hashing(const char* str) // ���ڿ� x12345 �ؽ� ����
{
	unsigned int hash = 0;
	while (*str) {
		hash = 12345 * hash + str[0];
		str++; // uppper pointer
	}
	return hash ^ (hash >> 16);
}

int getCmd(char* str, SOCKET tcp_sock) 
{
	char buf2[1024];
	memset(buf2, 0, 1024);
	int retval;
	unsigned int origin_cmd_hash = Hashing(str);
	SendCmd p;
	switch (origin_cmd_hash) {
	case(Hashing("exit")):
		strcpy(p.cmd, "exit");
		retval = send(tcp_sock, (char*)&p, sizeof(p), 0);
		closesocket(tcp_sock);
		WSACleanup();
		printf("bye :)\n");
		Sleep(5000);
		exit(0);
		break;
	case(Hashing("pwd")):
		strcpy(p.cmd, "pwd");
		retval = send(tcp_sock, (char*)&p, sizeof(p), 0);
		retval = recv(tcp_sock, buf2, sizeof(buf2), 0); // �о���� �������� ���� ��ȯ
		buf2[retval] = '\0';
		cout << buf2 << endl;
		break;
	case(Hashing("ls")):
		char tmp[512];
		int fsize;
		printf("type directory(relative or full path) : ");
		scanf("%s", tmp);
		strcpy(p.cmd, "ls");
		strcpy(p.argv, tmp);
		// FindFirstFile �Լ��� ���� ���ڱ���
		if (p.argv[strlen(p.argv) - 1] == '\\') 
			strcat(p.argv, "*");
		else
			strcat(p.argv, "\\*");
		retval = send(tcp_sock, (char*)&p, sizeof(p), 0); // ��ɰ� ���������� ������.
		recv(tcp_sock, buf2, sizeof(buf2), 0); // ó���� ������ ������ �޾ƿ�
		fsize = atoi(buf2);
		for (int i = 0; i < fsize; i++) {
			memset(buf2, 0, 1024);
			retval=recv(tcp_sock, buf2, sizeof(buf2), 0);
			buf2[retval] = '\0';
			printf("%s\n", buf2);
		}
		break;
	default:
		printf("Invalid Command\n");
		retval = 0;
	}
	return retval;
}

int main() {
	WSADATA wsa; // ������ ���� ������ ���� �� �ִ� ����ü
	int retval;
	

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN serv_addr;
	memset((void*)&serv_addr, 0x00, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8081);
	inet_pton(AF_INET, "127.0.0.1", &(serv_addr.sin_addr.s_addr));

	if (retval=connect(tcp_sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr))) {
		printf("can't connect");
		return 1;
	}
	else {
		printf("connection is Complete!\n");
	}

	//////////////////////////////////////////////////////////////
	/*		   Command �ޱ� ��, C&C �������� ���� �۾�			*/
	//////////////////////////////////////////////////////////////

	while (1) {
		printf("send command : ");
		char cmd[100]="";
		scanf("%s", cmd);
		getCmd(cmd, tcp_sock); // ��ɿ� ���� �з�
		printf("\n");
	}
	closesocket(tcp_sock);
	WSACleanup();
	return 0;
}