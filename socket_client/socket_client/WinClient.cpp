#include <winsock2.h>
#include <StdAfx.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <conio.h>

int getsocket()
{
	int hsock;
	int * p_int;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if (hsock == -1){
		printf("Error initializing socket %d\n", WSAGetLastError());
		return -1;
	}

	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if ((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1) ||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1)){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		return -1;
	}
	free(p_int);

	return hsock;
}

void SendCommand(char *command, struct sockaddr_in my_addr);

void DisplayServer(char *command, struct sockaddr_in my_addr)
{
	char screenname[32];
	int offset = 0;
	int i;
	for (i = 1; command[i] != '$'; i++)
		screenname[offset++] = command[i];
	screenname[offset] = '\0';
	i++;
	char printstring[1024];
	offset = 0;
	for (; command[i] != '$'; i++)
		printstring[offset++] = command[i];
	printstring[offset] = '\0';
	char min[10];
	offset = 0;
	i++;
	for (; command[i] != '$'; i++)
		min[offset++] = command[i];
	min[offset] = '\0';
	int Minimum = atoi(min);
	char max[10];
	offset = 0;
	i++;
	for (; command[i] != '$'; i++)
		max[offset++] = command[i];
	max[offset] = '\0';
	int Maximum = atoi(max);
	char getstring[32];
	offset = 0;
	i++;
	for (; command[i] != '$'; i++)
		getstring[offset++] = command[i];
	getstring[offset] = '\0';
	int getlength = strlen(getstring);
	char requestBuffer[1024];
	while (true)
	{
		system("CLS");
		printf("%s\n\t", printstring);
		printf("Enter your Choice\n\t");
		int option;
		char optionstring[32] = "@@@";
		scanf("%d", &option);
		if (option >= Minimum&&option <= Maximum)
		{
			if (getlength)
			{
				printf("\n\tEnter  %s\n\t", getstring);
				gets(optionstring);
				gets(optionstring);
			}
			char temp[5];
			itoa(option, temp, 10);
			strcpy(requestBuffer, "$");
			strcat(requestBuffer, screenname);
			strcat(requestBuffer, "$");
			strcat(requestBuffer, temp);
			strcat(requestBuffer, "$");
			strcat(requestBuffer, optionstring);
			strcat(requestBuffer, "$");
			SendCommand(requestBuffer, my_addr);
			return;
		}
	}
}


void DisplayServerStringRequest(char *command, struct sockaddr_in my_addr)
{

	int i;
	char screenname[32];
	int offset = 0;
	for (i = 1; command[i] != '$'; i++)
		screenname[offset++] = command[i];
	screenname[offset] = '\0';
	offset = 0;
	i++;
	char namestring[32];
	for (; command[i] != '$'; i++)
		namestring[offset++] = command[i];
	namestring[offset] = '\0';
	system("CLS");
	printf(" \n\tEnter %s\n\t", namestring);
	char inputstring[12];
	gets(inputstring);
	gets(inputstring);
	char requestBuffer[1024];
	strcpy(requestBuffer, "$");
	strcat(requestBuffer, screenname);
	strcat(requestBuffer, "$");
	strcat(requestBuffer, inputstring);
	strcat(requestBuffer, "$");
	SendCommand(requestBuffer, my_addr);
}


void ProcessServerRequest(char *command, struct sockaddr_in my_addr)
{
	if (command[0] == '@')
		DisplayServer(command, my_addr);
	else if (command[0] == '%')
	{
		system("CLS");
		printf("%s",command);
		getchar();
	}
	else{
		system("CLS");
		printf("%s", command);
		char query[200];
		char buffer[1024];
		printf("\n\tEnter the Query\n\t");
		gets(query);
		strcpy(buffer, "$query$");
		strcat(buffer, query);
		strcat(buffer, "$");
		SendCommand(buffer, my_addr);
	}
}

void SendCommand(char *command, struct sockaddr_in my_addr)
{
	char buffer[1024];
	int buffer_len = 1024;
	int bytecount;
	int c;
	strcpy(buffer, command);
	int hsock = getsocket();
	//add error checking on hsock...
	if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR){
		fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	printf("Sent bytes %d\n", bytecount);

	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	//printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
	char temp[1024];
	int i = 0;
	for (i = 0; buffer[i] != '#'; i++)
		temp[i] = buffer[i];
	temp[i] = '\0';
	ProcessServerRequest(temp, my_addr);
	closesocket(hsock);
}


void socket_client()
{

	//The port and address you want to connect to
	int host_port = 1101;
	char* host_name = "127.0.0.1";

	//Initialize socket support WINDOWS ONLY!
	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0 || (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)) {
		fprintf(stderr, "Could not find sock dll %d\n", WSAGetLastError());
		return;
	}
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(host_port);

	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

	SendCommand("$opened$", my_addr);

}