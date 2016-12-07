#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "messageTypes.h"
#include "messageStructs.h"

void initializeDatabase();
void createMessageQueues();
void DBServer();
void writeToDatabase(my_message msg);
my_message readFromDatabase();
int sendMessageToDBServer(my_message msg);
int receiveMessageFromDBServer(my_message msg);
int receiveMessageFromDBEditor(my_message msg);
int withdrawFromAccount(my_message msg);
void blockAccount(my_message mg);
void encodePIN(char originalPin[3],char encodedPin[3]);
void decodePIN(char originalPin[3],char decodedPin[3]);

//Fils names
//char *DBFilename = "DB.txt";

//Database 
typedef struct Account{
	char accountNumber[5];
	char encodedPIN[3];
	float fundsAvailable; 
}Account;

Account database[5]; // The bank allows 5 accounts. It's a pretty small bank 

//Variables 
int attempts = 0;
char * prevAccount;
int ATMServerMsgqid;
int ServerATMMsgqid;
int ServerEditorMsgqid;
int trials = 0;

int main (void){
	
	createMessageQueues();
	initializeDatabase();
	DBServer();
	
	return 1;
}

void DBServer(){
	while(1){
		//receive all from atm
		my_message ATMMessage;
		
		if(receiveMessageFromDBServer(ATMMessage) == -1){
			//If receiving failed print error and exit
			perror("msgrcv: msgrcv from atm failed\n");
			exit(1);
		}
		else{
			if(trials >= 3){
				blockAccount(ATMMessage);
				printf("Sorry out of trials");
				
			}
				
			//switch case
			switch(ATMMessage.message_type){ //act acording to each case
				case 1: //pin message
					printf("PIN Message Received From ATM\n");
					int accountExists = 0;
					char decodedPin[3];
					int i;
					trials +=1;
					//check if ok or not okay
					for(i = 0; i < sizeof(database); i++){
						if(ATMMessage.accountInfo.accountNum == database[i].accountNumber){
							decodePIN(ATMMessage.accountInfo.pin,decodedPin);
							if(strcmp(database[i].encodedPIN, decodedPin) == 0){
								printf("OK");
								accountExists = 1;
							}
							
						}
					}
					if(accountExists == 0){
						printf("NOT OK");
					}
							
					//and respond accordingly
					//TODO
					break;
				case 4: //request funds
					printf("Request Funds Message Received From ATM\n");
					my_message fundsMessage = readFromDatabase(ATMMessage);
					if(sendMessageToDBServer(fundsMessage) == -1){
						perror("msgsnd: msgsnd to ATM failed\n");
						exit(1);
					}
					else{
						printf("msgsnd: msgsnd to ATM sucess\n");
					}
					break;
				case 5: //withdraw
					printf("Withdraw Message Received From ATM\n");
					int result = withdrawFromAccount(ATMMessage);
					
					my_message sendMsg;
					sendMsg.message_type = result;
					
					if(sendMessageToDBServer(sendMsg) == -1){
							perror("msgsnd: msgsnd to ATM failed\n");
							exit(1);
					}
					else{
							printf("msgsnd: msgsnd to ATM sucess\n");
					}
					break;
				
			}
		}
		
		//receive all from editor
		my_message UpdateMessage;
		if(receiveMessageFromDBEditor(UpdateMessage) == -1){
			//If receiving failed print error and exit
			perror("msgrcv: msgrcv from editor failed\n");
			exit(1);
		}
		else{
			//if received, edit file
			printf("UpdateDB Message Received From Editor\n");
			writeToDatabase(UpdateMessage);
			printf("Edited Database Successfully\n");
			
		}
	}
}

//initialize the DB with the given accounts
void initializeDatabase(){
	Account account1, account2, account3;
	strcpy(account1.accountNumber,"00001");
	strcpy(account1.encodedPIN,"107");
	account1.fundsAvailable = 3443.22;
	
	strcpy(account2.accountNumber,"00011");
	strcpy(account2.encodedPIN,"323");
	account2.fundsAvailable = 10089.97;
	
	strcpy(account3.accountNumber,"00117");
	strcpy(account3.encodedPIN,"259");
	account3.fundsAvailable = 112.00;
	
	database[0] = account1;
	database[1] = account2;
	database[2] = account3;
	
}

void createMessageQueues(){
	ATMServerMsgqid = msgget((key_t)1234, IPC_CREAT| 0600);
	if(ATMServerMsgqid == -1){
		perror("msgget: ATMServerMsgqid failed\n");
		exit(1);
	}
	ServerATMMsgqid = msgget((key_t)12345, IPC_CREAT| 0600);
	if(ServerATMMsgqid == -1){
		perror("msgget: ServerATMMsgqid failed\n");
		exit(1);
	}
	ServerEditorMsgqid = msgget((key_t)123456, IPC_CREAT| 0600);
	if(ServerEditorMsgqid == -1){
		perror("msgget: ATMServerMsgqid failed\n");
		exit(1);
	}
}

void writeToDatabase(my_message msg){
	
}

//i doubt this works
my_message readFromDatabase(my_message msg){
	char line[100];
	char accountNum[5];
	char PINnumber[3];
	float funds;
	FILE * DB = fopen(DBFilename,"r");
	PINMessage pinMessage;
	my_message result;
	
	for(;fgets(line, sizeof(line), DB) !=NULL;){
		fgets(line, sizeof(line), DB);
		sscanf(line, "%s\t%s\t%f\n", accountNum, PINnumber,&funds);
		if(strcmp(msg.accountInfo.accountNum,accountNum)){
			strcpy(pinMessage.accountNum,accountNum);
			strcpy(pinMessage.pin, PINnumber);
			result.accountInfo = pinMessage;
			result.funds = funds;
			break;
		}
	}
	return result;
}

//return 8 if enough
//return 7 is not enough
int withdrawFromAccount(my_message msg){
	return 1;
}

int sendMessageToDBServer(my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgsnd(ServerATMMsgqid, &msg, msgLength,0);
}
int receiveMessageFromDBServer(my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgrcv(ATMServerMsgqid, &msg, msgLength, 0, 0);
}

int receiveMessageFromDBEditor(my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgrcv(ServerEditorMsgqid, &msg, msgLength, 6, 0);
}

void blockAccount(my_message msg){
	msg.accountInfo.accountNum[0] = 'X';
}

void encodePIN(char originalPin[3],char encodedPin[3]){
	int i;
	for(i = 0; i < sizeof(originalPin); i++){
		if(originalPin[i] == '9'){
			encodedPin[i] = '0';
		}
		else{
			encodedPin[i] = originalPin[i] +1;
		} 
	}
}
	
void decodePIN(char originalPin[3],char decodedPin[3]){
	
	int i;
	for(i = 0; i < sizeof(originalPin); i++){
		if(originalPin[i] == '0'){
			decodedPin[i] = '9';
		}
		else{
			decodedPin[i] = originalPin[i] -1;
		} 
	}
}
