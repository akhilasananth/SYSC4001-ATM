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
void writeToDatabase();
my_message readFromDatabase();
int sendMessageToDBServer(my_message msg);
int receiveMessageFromDBServer(my_message msg);
int receiveMessageFromDBEditor(my_message msg);
int withdrawFromAccount(my_message msg);
void blockAccount(my_message mg);
int searchDatabase(my_message msg);
void encodePIN(char originalPin[3],char encodedPin[3]);
void decodePIN(char originalPin[3],char decodedPin[3]);
int getSize();

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
	printf("Starting Server...\n");
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
			//switch case
			printf("Message Received\n");
			printf("Message type: %ld\n",ATMMessage.message_type);
			printf("Funds: %f\n",ATMMessage.funds);
			switch(ATMMessage.message_type){ //act acording to each case
				case 1: //pin message
					printf("PIN Message Received From ATM\n");
					int i = searchDatabase(ATMMessage);
					if(i == -1){
						my_message notOkMessage;
						notOkMessage.message_type = notOk;
						notOkMessage.accountInfo = ATMMessage.accountInfo;
						if(sendMessageToDBServer(notOkMessage) == -1){
							perror("msgsnd: msgsnd to ATM failed\n");
							exit(1);
						}
						else{
							printf("msgsnd: msgsnd to ATM sucess\n");
						}
						trials +=1;
						if(trials == 3){
							blockAccount(ATMMessage);
						}
					}
					else{
						char decodedPin[3];
						decodePIN(database[i].encodedPIN,decodedPin);
						if(strcmp(ATMMessage.accountInfo.pin, decodedPin) == 0){
							my_message okMessage;
							okMessage.message_type = ok;
							okMessage.accountInfo = ATMMessage.accountInfo;
							if(sendMessageToDBServer(okMessage) == -1){
								perror("msgsnd: msgsnd to ATM failed\n");
								exit(1);
							}
							else{
								printf("msgsnd: msgsnd to ATM sucess\n");
							}
							continue;
						}
					}
					break;
				case 4: //request funds
					printf("Request Funds Message Received From ATM\n");
					my_message fundsMessage;
					fundsMessage.accountInfo = ATMMessage.accountInfo;
					fundsMessage.funds = database[searchDatabase(ATMMessage)].fundsAvailable;
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
					sendMsg.accountInfo = ATMMessage.accountInfo;
					
					if(sendMessageToDBServer(sendMsg) == -1){
							perror("msgsnd: msgsnd to ATM failed\n");
							exit(1);
					}
					else{
							printf("msgsnd: msgsnd to ATM sucess\n");
					}
					break;
				default:
					printf("Default Option\n");
					break;
			}
			printf("now what?\n");
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
			Account newAccount;
			//TODO
			database[getSize(database)+1] = newAccount;
			writeToDatabase();
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
	writeToDatabase();
	
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

void writeToDatabase(){
	FILE *output = fopen("DB.txt", "wb");
	if(!output){
		perror("Error while opening file");
		exit(0);
	}
	int i;
	for(i = 0; i<getSize(database);++i){
		fprintf(output, "%s\t%s\t%f\n",database[i].accountNumber, database[i].encodedPIN, database[i].fundsAvailable);
	}
	fclose(output);
}

int withdrawFromAccount(my_message msg){
	int i = searchDatabase(msg);
	if(database[i].fundsAvailable < msg.withdrawAmount){
		return notEnoughFunds;
	}
	else{
		database[i].fundsAvailable -= msg.withdrawAmount;
		writeToDatabase();
		return enoughFunds;
	}
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
	int i = searchDatabase(msg);
	database[i].accountNumber[0] = 'X';
	writeToDatabase();
}

//return -1 if not found
int searchDatabase(my_message msg){
	int i;
	for(i = 0; i < getSize(database); i++){
		if(strcmp(msg.accountInfo.accountNum,database[i].accountNumber) == 0){
			return i;
		}
	}
	return -1;
}

void encodePIN(char originalPin[3],char encodedPin[3]){
	int i;
	for(i = 0; i < 3; i++){
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
	for(i = 0; i < 3; i++){
		if(originalPin[i] == '0'){
			decodedPin[i] = '9';
		}
		else{
			decodedPin[i] = originalPin[i] -1;
		} 
	}
}

int getSize(){
	return (sizeof(database)/sizeof(Account));
}
