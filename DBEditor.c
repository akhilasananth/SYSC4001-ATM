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
int checkIfNumbersANum(char arr[5]);
int checkIfNumbersPin(char arr[3]);

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
	fundsInput = (char *)malloc(50);
		while(1){
			
			printf("Enter your 5 digit account number: ");
			scanf("%s", editorInput.accountInfo.accountNum);
			
			checkForExit(editorInput.accountInfo.accountNum);
			
			
			if((strlen(editorInput.accountInfo.accountNum) != 5) || (checkIfNumbersANum(editorInput.accountInfo.accountNum)==0)){
				perror("Invalid entry\n");
				continue;
			}
			editorInput.accountInfo.accountNum[5] = '\0';
			
			do{
				printf("Enter your 3 digit PIN number: ");
				scanf("%s", editorInput.accountInfo.pin);
				editorInput.accountInfo.pin[3] = '\0';
				checkForExit(editorInput.accountInfo.pin);
			}while(strlen(editorInput.accountInfo.pin) != 3 || checkIfNumbersPin(editorInput.accountInfo.pin)==0);

			do{ 
				printf("Enter funds available: ");
				scanf("%f", &editorInput.funds);
				sprintf(fundsInput,"%f",editorInput.funds);
				checkForExit(fundsInput);
			}while(editorInput.funds< 0); //Checks for negative money
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

int checkIfNumbersANum(char arr[5]){
	int i;
	int ret = 1;
	for(i = 0; i< 5; i++){
		ret = isdigit(arr[i]);
		if(ret == 0){
			break;
		}
	}
	
	return ret;
}

int checkIfNumbersPin(char arr[3]){
	int i;
	int ret = 1;
	for(i = 0; i< 3; i++){
		ret = isdigit(arr[i]);
		if(ret == 0){
			break;
		}
	}
	
	return ret;
}

		
