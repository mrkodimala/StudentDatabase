#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist,*alist,*printid;

struct bufserv{
	
		int userId;
		int forumId;
		int msgId;
		int commentId;
		int choice;
		char *forumname;
		char msg[128];
}buf1;

bool flag=true;
int mid = 0;
int count1 =0;
char *Data[100];
int count=1;
int values[100];
DWORD WINAPI SocketHandler(void*);
void replyto_client(char *buf, int *csock);

void socket_server() {

	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff

	int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	
	while(true){
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));
		
		if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET ){
			//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			CreateThread(0,0,&SocketHandler, (void*)csock , 0,0);
		}
		else{
			fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
		}
	}

FINISH:
;
}

struct StudentDetails{
	int student_id;
	char name[32];
	char city[32];
	struct StudentDetails *next;
};

struct StudentMarks{
	int student_id;
	int marks[6];
	int total;
	struct StudentMarks *next;
};

struct StudentDetails *SDhead;
struct StudentMarks *SMhead;


void SeparateStudentDetail(char *buffer, char *name, char *city, int *studentid)
{
	int i;
	int offset = 0;
	char temp[5];
	for (i = 0; buffer[i] != ','; i++)
		temp[offset++] = buffer[i];
	temp[offset] = '\0';
	*studentid = atoi(temp);
	i++;
	offset = 0;
	for (; buffer[i] != ','; i++)
		name[offset++] = buffer[i];
	name[offset] = '\0';
	i++;
	offset = 0;
	for (; buffer[i] != '\n'&&buffer[i] != '\0'; i++)
		city[offset++] = buffer[i];
	city[offset] = '\0';
}

struct StudentDetails * ReverseSLL1(struct StudentDetails *head)
{
	struct StudentDetails *temp;
	if (head->next == NULL)
		return head;
	else{
		temp = ReverseSLL1(head->next);
		head->next->next = head;
		head->next = NULL;
		return temp;
	}
}

struct StudentDetails * CreateDetailNode(int student_id, char *name, char *city)
{
	struct StudentDetails *newNode = (struct StudentDetails *)malloc(sizeof(struct StudentDetails));
	newNode->student_id = student_id;
	strcpy(newNode->name, name);
	strcpy(newNode->city, city);
	newNode->next = NULL;
	return newNode;
}

void InsertIntoStudentDetailsSll(int student_id, char *name, char *city)
{
	struct StudentDetails *temp = CreateDetailNode(student_id, name, city);
	if (SDhead == NULL)
	{
		SDhead = temp;
	}
	else{
		temp->next = SDhead;
		SDhead = temp;
	}
}

void ReadStudentDetails()
{
	FILE *file = fopen("studentdetails.csv", "r");
	char buffer[128];
	char name[32], city[32];
	int student_id;
	while (!feof(file))
	{
		fgets(buffer, 128, file);
		SeparateStudentDetail(buffer, name, city, &student_id);
		InsertIntoStudentDetailsSll(student_id, name, city);
	}
	SDhead = ReverseSLL1(SDhead);
	fclose(file);
}

void DisplayStudentDetails()
{
	struct StudentDetails *temp = SDhead;
	while (temp != NULL)
	{
		printf("%d\t%s\t%s\n", temp->student_id, temp->name, temp->city);
		temp = temp->next;
	}
}

void SeparateStudentMarks(char *buffer, int *marks,int *student_id)
{
	int i;
	char temp[5];
	int offset = 0;
	for (i = 0; buffer[i] != ','; i++)
		temp[offset++] = buffer[i];
	temp[offset] = '\0';
	*student_id = atoi(temp);
	count = 0;
	while (count < 6)
	{
		i++;
		offset = 0;
		for (; buffer[i] != ','&&buffer[i] != '\n'&&buffer[i] != '\0'; i++)
			temp[offset++] = buffer[i];
		temp[offset] = '\0';
		marks[count++] = atoi(temp);
	}
}

struct StudentMarks * CreateMarksNode(int student_id, int *marks)
{
	struct StudentMarks *newNode = (struct StudentMarks *)malloc(sizeof(struct StudentMarks));
	newNode->student_id = student_id;
	int sum = 0;
	for (int i = 0; i < 6; i++)
	{
		sum += marks[i];
		newNode->marks[i] = marks[i];
	}
	newNode->total = sum;
	newNode->next = NULL;
	return newNode;
}

void InsertIntoStudentMarksSLL(int student_id, int *marks)
{
	struct StudentMarks *temp = CreateMarksNode(student_id, marks);
	if (SMhead == NULL)
	{
		SMhead = temp;
	}
	else{
		temp->next = SMhead;
		SMhead = temp;
	}
}

struct StudentMarks * ReverseSLL2(struct StudentMarks *head)
{
	struct StudentMarks *temp;
	if (head->next == NULL)
		return head;
	else{
		temp = ReverseSLL2(head->next);
		head->next->next = head;
		head->next = NULL;
		return temp;
	}
}

void ReadStudentMarks()
{
	FILE *file = fopen("studentmarks.csv", "r");
	int marks[6];
	int student_id;
	char buffer[128];
	while (!feof(file))
	{
		fgets(buffer, 128, file);
		SeparateStudentMarks(buffer, marks,&student_id);
		InsertIntoStudentMarksSLL(student_id, marks);
	}
	SMhead = ReverseSLL2(SMhead);
	fclose(file);
}

void DisplayStudentMarks()
{
	struct StudentMarks *temp = SMhead;
	while (temp != NULL)
	{
		printf("%d\t", temp->student_id);
		for (int i = 0; i < 6; i++)
			printf("%d\t", temp->marks[i]);
		printf("%d\n",temp->total);
		temp = temp->next;
	}
}

void ReadDataFromFilesAndFormSLLs()
{
	SDhead = NULL;
	SMhead = NULL;
	ReadStudentDetails();
	ReadStudentMarks();
	//DisplayStudentDetails();
	//DisplayStudentMarks();
}


struct Tokens{
	char token[30];
};

struct Tokens collection[50];
int collection_count;

struct Tokens instructions[12];
int ins_count;


struct Operators{
	char ops[5];
};

struct Operators operators[8];
int ops_count;

void LoadInstructionsIntoInstructions()
{
	ins_count = 0;
	strcpy(instructions[ins_count++].token, "student_id");
	strcpy(instructions[ins_count++].token, "name");
	strcpy(instructions[ins_count++].token, "city");
	strcpy(instructions[ins_count++].token, "total");
	strcpy(instructions[ins_count++].token, "django");
	strcpy(instructions[ins_count++].token, "backbone");
	strcpy(instructions[ins_count++].token, "cprogramming");
	strcpy(instructions[ins_count++].token, "algorithms");
	strcpy(instructions[ins_count++].token, "angularjs");
	strcpy(instructions[ins_count++].token, "python");
}

void LoadOperatorsIntoOps()
{
	ops_count = 0;
	strcpy(operators[ops_count++].ops, ">");
	strcpy(operators[ops_count++].ops, ">=");
	strcpy(operators[ops_count++].ops, "<");
	strcpy(operators[ops_count++].ops, "<=");
	strcpy(operators[ops_count++].ops, "=");
	strcpy(operators[ops_count++].ops, "!=");
}


void TokenizeQuery(char *query)
{
	int length = strlen(query);
	int i = 0;
	char temp[30];
	int offset = 0;
	collection_count = 0;
	while (i < length)
	{
		for (; query[i] == ' ' || query[i] == '\t'; i++);
		for (; query[i] != ' '&&query[i] != '\0'; i++)
			temp[offset++] = query[i];
		temp[offset] = '\0';
		strcpy(collection[collection_count++].token, temp);
		offset = 0;
	}
}

void PrintTokens()
{
	for (int i = 0; i < collection_count; i++)
	{
		printf("%s\n", collection[i].token);
	}
}

int checkIfStringisInstruction(char *token)
{
	int i;
	for (i = 0; i < ins_count; i++)
	{
		if (!strcmp(token, instructions[i].token))
			return i;
	}
	return -1;
}

int checkifStringisOperator(char *token)
{
	for (int i = 0; i < ops_count; i++)
	{
		if (!strcmp(token, operators[i].ops))
			return i;
	}
	return -1;
}

int checkWhetherTokenisString(char *token)
{
	int length = strlen(token);
	if (token[0] == '\''&&token[length - 1] == '\'')
		return 1;
	return 0;
}

int checkWhetherTokenisInteger(char *token)
{
	int length = strlen(token);
	if (length == 1 && token[0] == '0')
		return 1;
	int c = atoi(token);
	if (c > 0)
		return 1;
	return 0;
}

struct ResultIds{
	int data;
	struct ResultIds *next;
};

struct ResultIds *Rhead;

struct ResultIds * CreateResultNode(int  value)
{
	struct ResultIds *newNode = (struct ResultIds *)malloc(sizeof(struct ResultIds));
	newNode->data = value;
	newNode->next = NULL;
	return newNode;
}

void InsertToResultHead(int value)
{
	struct ResultIds *temp = CreateResultNode(value);
	if (Rhead == NULL)
		Rhead = temp;
	else{
		temp->next = Rhead;
		Rhead = temp; 
	}
}


int PerfromQueryonDetailsSLL(char *column, char *oper, char *value, int operation_value, int no)
{
	int opvalue = checkifStringisOperator(oper);
	struct StudentDetails *temp = SDhead;
	char tempvalue[32];
	int offset = 0;
	int length = strlen(value);
	for (int i = 1; i < length - 1; i++)
		tempvalue[offset++] = value[i];
	tempvalue[offset] = '\0';
	if (opvalue == 4)
	{
		if (operation_value == 2||operation_value==0)
		{
			while (temp != NULL)
			{
				if (no == 1)
				{
					if (!strcmp(temp->name, tempvalue))
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no == 2){
					if (!strcmp(temp->city, tempvalue))
					{
						InsertToResultHead(temp->student_id);
					}
				}
				temp = temp->next;
			}
		}
		else if(operation_value==1){
			struct ResultIds *tempresult = Rhead;
			Rhead = NULL;
			while (temp != NULL)
			{
				if (tempresult == NULL)
					return 0;
				if (temp->student_id == tempresult->data)
				{
					if (no == 1)
					{
						if (!strcmp(temp->name, tempvalue))
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no == 2){
						if (!strcmp(temp->city, tempvalue))
						{
							InsertToResultHead(temp->student_id);
						}
					}
					tempresult = tempresult->next;
				}
				temp = temp->next;
			}
		}
	}
	else if (opvalue == 5){
		if (operation_value == 2 || operation_value == 0)
		{
			while (temp != NULL)
			{
				if (no == 1)
				{
					if (strcmp(temp->name, tempvalue))
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no == 2){
					if (strcmp(temp->city, tempvalue))
					{
						InsertToResultHead(temp->student_id);
					}
				}
				temp = temp->next;
			}
		}
		else if (operation_value == 1){
			struct ResultIds *tempresult = Rhead;
			Rhead = NULL;
			while (temp != NULL)
			{
				if (tempresult == NULL)
					return 0;
				if (temp->student_id == tempresult->data)
				{
					if (no == 1)
					{
						if (strcmp(temp->name, tempvalue))
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no == 2){
						if (strcmp(temp->city, tempvalue))
						{
							InsertToResultHead(temp->student_id);
						}
					}
					tempresult = tempresult->next;
				}
				temp = temp->next;
			}
		}
	}
	return 0;
}

int PerformQueryonMarksSLL(char *column, char *oper, char *value, int operation_value, int no)
{
	int opvalue = checkifStringisOperator(oper);
	struct StudentMarks *temp = SMhead;
	int tempvalue = atoi(value);
	if (opvalue == 4)
	{
		if (operation_value == 2 || operation_value == 0)
		{
			while (temp != NULL)
			{
				if (no == 0)
				{
					if (temp->student_id==tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no == 3){
					if (temp->total==tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no >= 4 && no <= 9){
					if (temp->marks[no - 4] == tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				temp = temp->next;
			}
		}
		else if (operation_value == 1){
			struct ResultIds *tempresult = Rhead;
			Rhead = NULL;
			while (temp != NULL)
			{
				if (tempresult == NULL)
					return 0;
				if (temp->student_id == tempresult->data)
				{
					if (no == 0)
					{
						if (temp->student_id == tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no == 3){
						if (temp->total == tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no >= 4 && no <= 9){
						if (temp->marks[no - 4] == tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					tempresult = tempresult->next;
				}
				temp = temp->next;
			}
		}
	}
	else if (opvalue == 5)
	{
		if (operation_value == 2 || operation_value == 0)
		{
			while (temp != NULL)
			{
				if (no == 0)
				{
					if (temp->student_id != tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no == 3){
					if (temp->total != tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no >= 4 && no <= 9){
					if (temp->marks[no - 4] != tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				temp = temp->next;
			}
		}
		else if (operation_value == 1){
			struct ResultIds *tempresult = Rhead;
			Rhead = NULL;
			while (temp != NULL)
			{
				if (tempresult == NULL)
					return 0;
				if (temp->student_id != tempresult->data)
				{
					if (no == 0)
					{
						if (temp->student_id != tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no == 3){
						if (temp->total != tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no >= 4 && no <= 9){
						if (temp->marks[no - 4] != tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					tempresult = tempresult->next;
				}
				temp = temp->next;
			}
		}
	}
	else if (opvalue == 0)
	{
		if (operation_value == 2 || operation_value == 0)
		{
			while (temp != NULL)
			{
				if (no == 0)
				{
					if (temp->student_id > tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no == 3){
					if (temp->total > tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no >= 4 && no <= 9){
					if (temp->marks[no - 4] > tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				temp = temp->next;
			}
		}
		else if (operation_value == 1){
			struct ResultIds *tempresult = Rhead;
			Rhead = NULL;
			while (temp != NULL)
			{
				if (tempresult == NULL)
					return 0;
				if (temp->student_id != tempresult->data)
				{
					if (no == 0)
					{
						if (temp->student_id > tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no == 3){
						if (temp->total > tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no >= 4 && no <= 9){
						if (temp->marks[no - 4] > tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					tempresult = tempresult->next;
				}
				temp = temp->next;
			}
		}
	}
	else if (opvalue == 1)
	{
		if (operation_value == 2 || operation_value == 0)
		{
			while (temp != NULL)
			{
				if (no == 0)
				{
					if (temp->student_id >= tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no == 3){
					if (temp->total >= tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no >= 4 && no <= 9){
					if (temp->marks[no - 4] >= tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				temp = temp->next;
			}
		}
		else if (operation_value == 1){
			struct ResultIds *tempresult = Rhead;
			Rhead = NULL;
			while (temp != NULL)
			{
				if (tempresult == NULL)
					return 0;
				if (temp->student_id != tempresult->data)
				{
					if (no == 0)
					{
						if (temp->student_id >= tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no == 3){
						if (temp->total >= tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no >= 4 && no <= 9){
						if (temp->marks[no - 4] >= tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					tempresult = tempresult->next;
				}
				temp = temp->next;
			}
		}
	}
	else if (opvalue == 2)
	{
		if (operation_value == 2 || operation_value == 0)
		{
			while (temp != NULL)
			{
				if (no == 0)
				{
					if (temp->student_id < tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no == 3){
					if (temp->total < tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no >= 4 && no <= 9){
					if (temp->marks[no - 4] < tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				temp = temp->next;
			}
		}
		else if (operation_value == 1){
			struct ResultIds *tempresult = Rhead;
			Rhead = NULL;
			while (temp != NULL)
			{
				if (tempresult == NULL)
					return 0;
				if (temp->student_id != tempresult->data)
				{
					if (no == 0)
					{
						if (temp->student_id < tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no == 3){
						if (temp->total < tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no >= 4 && no <= 9){
						if (temp->marks[no - 4] < tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					tempresult = tempresult->next;
				}
				temp = temp->next;
			}
		}
	}
	else if (opvalue == 3)
	{
		if (operation_value == 2 || operation_value == 0)
		{
			while (temp != NULL)
			{
				if (no == 0)
				{
					if (temp->student_id <= tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no == 3){
					if (temp->total <= tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				else if (no >= 4 && no <= 9){
					if (temp->marks[no - 4] <= tempvalue)
					{
						InsertToResultHead(temp->student_id);
					}
				}
				temp = temp->next;
			}
		}
		else if (operation_value == 1){
			struct ResultIds *tempresult = Rhead;
			Rhead = NULL;
			while (temp != NULL)
			{
				if (tempresult == NULL)
					return 0;
				if (temp->student_id != tempresult->data)
				{
					if (no == 0)
					{
						if (temp->student_id <= tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no == 3){
						if (temp->total <= tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					else if (no >= 4 && no <= 9){
						if (temp->marks[no - 4] <= tempvalue)
						{
							InsertToResultHead(temp->student_id);
						}
					}
					tempresult = tempresult->next;
				}
				temp = temp->next;
			}
		}
	}
	return 0;
}

void DisplayResultIds()
{
	struct ResultIds *temp = Rhead;
	while (temp != NULL)
	{
		printf("\nresultid=%d", temp->data);
		temp = temp->next;
	}
}

int PerformQueryOperationOnData(char *column, char *oper, char *value, int operation_value)
{
	int flag = checkIfStringisInstruction(column);
	if (flag == 1 || flag == 2)
	{
		PerfromQueryonDetailsSLL(column, oper, value, operation_value,flag);
	}
	else{
		PerformQueryonMarksSLL(column, oper, value, operation_value, flag);
	}
	return 0;
}

struct ResultIds * ReverseSLL3(struct ResultIds *head)
{
	if (head == NULL)
		return NULL;
	struct ResultIds *temp;
	if (head->next == NULL)
		return head;
	else{
		temp = ReverseSLL3(head->next);
		head->next->next = head;
		head->next = NULL;
		return temp;
	}
}


struct ResultIds * RemoveDuplicates(struct ResultIds *head)
{
	struct ResultIds *temp = head;
	int Arr[100];
	int offset = 0;
	if (head == NULL)
		return NULL;
	Arr[offset++] = head->data;
	int flag = 0;
	while (head ->next!= NULL)
	{
		flag = 0;
		for (int i = 0; i < offset; i++)
		{
			if (head->next->data == Arr[i])
				flag = 1;
		}
		if (flag == 1)
		{
			head->next = head->next->next;
		}
		else{
			Arr[offset++] = head->next->data;
			head = head->next;
		}
	}
	return temp;
}


int CheckWhetherTokens()
{
	int i=0;
	Rhead = NULL;
	if (collection_count < 2)
		return 0;
	while (i<collection_count)
	{
		int k1 = checkIfStringisInstruction(collection[i].token);
		int k2 = checkifStringisOperator(collection[i + 1].token);
		if (k1 == -1 || k2 == -1)
			return 0;
		int k3;
		if (k1 == 1 || k1 == 2)
		{
			k3 = checkWhetherTokenisString(collection[i + 2].token);
		}
		else{
			k3 = checkWhetherTokenisInteger(collection[i + 2].token);
		}
		if (k3 <= 0)
			return 0;
		if (i - 1 > 0)
		{
			int o;
			if (!strcmp(collection[i -1].token, "&&"))
				o = 1;
			else if (!strcmp(collection[i -1].token, "||"))
				o = 2;
			else
				return 0;
			PerformQueryOperationOnData(collection[i].token, collection[i + 1].token, collection[i + 2].token, o);
		}
		else{
			PerformQueryOperationOnData(collection[i].token, collection[i + 1].token, collection[i + 2].token, 0);
		}
		Rhead=ReverseSLL3(Rhead);
		Rhead = RemoveDuplicates(Rhead);
		DisplayResultIds();
		i = i + 4;
	}
	return 1;
}


void ProcessQuery(char *command)
{
	int i;
	for (i = 1; command[i] != '$'; i++);
	char query[128];
	i++;
	int offset = 0;
	for (; command[i] != '$'; i++)
		query[offset++] = command[i];
	query[offset] = '\0';
	TokenizeQuery(query);
	LoadInstructionsIntoInstructions();
	LoadOperatorsIntoOps();
	int flag=CheckWhetherTokens();
	if (flag == 0)
		printf("There is an Error in the query\n");
	else{
		printf("flag=%d", flag);
		DisplayResultIds();
	}
}


void ProcessOutput(char *command)
{
	struct ResultIds *rhead = Rhead;
	struct StudentDetails *sdhead = SDhead;
	struct StudentMarks *smhead = SMhead;
	if (rhead == NULL)
	{
		strcpy(command, "No Rows Matched with your query#");
	}
	else{
		strcpy(command, "%\n\tResult of the Query\n\t");
		while (rhead != NULL)
		{
			while (rhead->data != sdhead->student_id)
			{
				sdhead = sdhead->next;
				smhead=smhead->next;
			}
			char temp[5];
			itoa(sdhead->student_id, temp, 10);
			strcat(command, temp);
			strcat(command, "\t");
			strcat(command, sdhead->name);
			strcat(command, "\t");
			strcat(command, sdhead->city);
			strcat(command, "\t");
			for (int i = 0; i < 6; i++)
			{
				itoa(smhead->marks[0], temp, 10);
				strcat(command, temp);
				strcat(command, "\t");
			}
			itoa(smhead->total, temp, 10);
			strcat(command, temp);
			strcat(command, "\n\t");
			rhead = rhead->next;
		}
		strcat(command, "#");
	}
}


int processrecvbuf(char *command)
{
	if (command[0] != '$')
		return 0;
	char buffer[32];
	int offset = 0;
	for (int i = 1; command[i] != '$'; i++)
		buffer[offset++] = command[i];
	buffer[offset] = '\0';
	if (!strcmp("opened", buffer))
		return 1;
	else if (!strcmp("query", buffer))
		return 2;
}

void process_input(char *recvbuf, int recv_buf_cnt, int* csock)
{

	char replybuf[1024] = { '\0' };
	int k = processrecvbuf(recvbuf);
	if (k == 1)
	{
		ReadDataFromFilesAndFormSLLs();
		printf("Client is started\n");
		strcpy(recvbuf, "\n\tQuery will return all the student Details\n\n\tQuery Structure  : column_name coditionn\n\tExample : Total > 500 && city = 'Hyderabad' \n\t Space is must Between All The words\n\t Like Space Between Total  and '='\n\t ");
		strcat(recvbuf, "\n\tColumn Names Allowed\n\tstudent_id\tname\tcity\ttotal\n\tSubject Names Allowed:\n\tdjango\tbackbone\tcprogramming\talgorithms\tangularjs\tpython\n\t#");
	}
	else if (k == 2)
	{
		printf("Received Buffer : %s\n", recvbuf);
		ProcessQuery(recvbuf);
		ProcessOutput(recvbuf);
		//strcpy(recvbuf, "%#");
	}
	replyto_client(recvbuf, csock);
	replybuf[0] = '\0';
}

void replyto_client(char *buf, int *csock) {
	int bytecount;
	
	if((bytecount = send(*csock, buf, strlen(buf), 0))==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free (csock);
	}
	printf("replied to client: %s\n",buf);
}

DWORD WINAPI SocketHandler(void* lp){
    int *csock = (int*)lp;

	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;

	memset(recvbuf, 0, recvbuf_len);
	if((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0))==SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free (csock);
		return 0;
	}

	//printf("Received bytes %d\nReceived string \"%s\"\n", recv_byte_cnt, recvbuf);
	process_input(recvbuf, recv_byte_cnt, csock);

    return 0;
}