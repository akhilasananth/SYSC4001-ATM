#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "messageTypes.h"
#include "messageStructs.h"
#include <string.h>
#include <ctype.h>

void checkForExit(char* userInput);
int checkIfNumbers(char* arr);

//Variables
int ServerEditorMsgqid;
char* fundsInput;
int main (void){
	
	printf("Starting DBEditor...\n Type \"X\" to exit\n");
	ServerEditorMsgqid = msgget((key_t)123456, IPC_CREAT| 0600);
	if(ServerEditorMsgqid == -1){
		perror("msgget: ATMServerMsgqid failed\n");
		exit(1);
	}
	while(1){
	//Get account information
	my_message editorInput;
	fundsInput = (char *)malloc(100);
		while(1){
			
			printf("Enter your 5 digit account number: ");
			scanf("%s", editorInput.accountInfo.accountNum);
			editorInput.accountInfo.accountNum[5] = '\0';
			if(strcmp(editorInput.accountInfo.accountNum,"X")==0 || strcmp(editorInput.accountInfo.accountNum,"x")==0 ){
				printf("Closing ATM\n");
				exit(1);
			}
			
			
			if(strlen(editorInput.accountInfo.accountNum) != 5 || checkIfNumbers(editorInput.accountInfo.accountNum)==0){
				perror("Invalid entry\n");
				continue;
			}

			printf("Enter your 3 digit PIN number: ");
			scanf("%s", editorInput.accountInfo.pin);
			editorInput.accountInfo.pin[3] = '\0';
			if(strcmp(editorInput.accountInfo.pin,"X")==0 || strcmp(editorInput.accountInfo.pin,"x")==0 ){
				printf("Closing ATM\n");
				exit(1);
			}

			while(strlen(editorInput.accountInfo.pin) != 3 || checkIfNumbers(editorInput.accountInfo.pin)==0){
				perror("Invalid entry\n");
				printf("Enter your 3 digit PIN number: ");
				scanf("%s", editorInput.accountInfo.pin);
				editorInput.accountInfo.pin[3] = '\0';
				if(strcmp(editorInput.accountInfo.pin,"X")==0 || strcmp(editorInput.accountInfo.pin,"x")==0 ){
					printf("Closing ATM\n");
					exit(1);
				}
				
			}
			 
			printf("Enter funds available: ");
			scanf("%f", &editorInput.funds);
			sprintf(fundsInput,"%f",editorInput.funds);
			checkForExit(fundsInput);

			while(editorInput.funds< 0 || !isdigit(editorInput.funds)){ //Checks for negative money
				printf("Enter funds available: ");
				scanf("%f", &editorInput.funds);
				sprintf(fundsInput,"%f",editorInput.funds);
				checkForExit(fundsInput);
			}
			break;
		}
		//********************************************************
	
		//Send DB Message to DB Server
			editorInput.message_type = updateDB;
			
			int msgLength = sizeof(my_message) - sizeof(long);
			if(msgsnd(ServerEditorMsgqid, &editorInput, msgLength,0) == -1){
				perror("msgsnd: msgsnd failed\n");
				exit(1);
			}
	}
	return 1;
}

void checkForExit(char* userInput){
	if(strcmp(userInput,"X")==0 || strcmp(userInput,"x")==0 ){
		printf("Closing ATM\n");
		exit(1);
	}
}

int checkIfNumbers(char* arr){
	int i;
	int ret = 1;
	for(i = 0; i< sizeof(arr)/sizeof(char); i++){
		if(isdigit(arr[i])){
			ret = 0;
		}
	}
	
	return ret;
}
		
