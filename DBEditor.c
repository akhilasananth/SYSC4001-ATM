#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "messageTypes.h"
#include "messageStructs.h"
#include <string.h>

my_message requestEditor();
void DBEditor();
int sendMessageToDBServer(my_message msg);
void checkForExit(char * userInput);

//Variables
int ServerEditorMsgqid;

int main (void){
	
	ServerEditorMsgqid = msgget((key_t)123456, IPC_CREAT| 0600);
	if(ServerEditorMsgqid == -1){
		perror("msgget: ATMServerMsgqid failed\n");
		exit(1);
	}
	DBEditor();
	return 1;
}


void DBEditor(){
	while(1){
	printf("Starting DBEditor...\n Type \"X\" to exit\n");
	//Get account information
	my_message aInfo;
	aInfo = requestEditor();
	
	//Send DB Message to DB Server
		aInfo.message_type = updateDB;
		
		if(sendMessageToDBServer(aInfo) == -1){
			perror("msgsnd: msgsnd failed\n");
			exit(1);
		}
		else{
			printf("msgsnd: UpdateDB sucess\n");
		}
	}
}

my_message requestEditor(){
	my_message editorInput;
	char* fundsInput = "";
	while(1){
		
		printf("Enter your 5 digit account number: ");
		scanf("%s", editorInput.accountInfo.accountNum);
		editorInput.accountInfo.accountNum[5] = '\0';
		checkForExit(editorInput.accountInfo.accountNum);
		
		if(strlen(editorInput.accountInfo.accountNum) != 5){
			perror("Invalid entry\n");
			continue;
		}

		printf("Enter your 3 digit PIN number: ");
		scanf("%s", editorInput.accountInfo.pin);
		editorInput.accountInfo.pin[3] = '\0';

		checkForExit(editorInput.accountInfo.pin);

		while(strlen(editorInput.accountInfo.pin) != 3){
			perror("Invalid entry\n");
			printf("Enter your 3 digit PIN number: ");
			scanf("%s", editorInput.accountInfo.pin);
			editorInput.accountInfo.pin[3] = '\0';

			checkForExit(editorInput.accountInfo.pin);
			
		}
		 
		printf("Enter funds available: ");
		scanf("%f", &editorInput.funds);
		sprintf(fundsInput,"%f",editorInput.funds);
		checkForExit(fundsInput);

		while(editorInput.funds< 0){ //Checks for negative money
			printf("Enter funds available: ");
			scanf("%f", &editorInput.funds);
			sprintf(fundsInput,"%f",editorInput.funds);
			checkForExit(fundsInput);
		}
		
		break;

	}
	return editorInput;
}

void checkForExit(char * userInput){
	if(strcmp(userInput,"X")==0 || strcmp(userInput,"x")==0 ){
		printf("Closing ATM\n");
		exit(1);
	}
}

int sendMessageToDBServer(my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgsnd(ServerEditorMsgqid, &msg, msgLength,0);
}

