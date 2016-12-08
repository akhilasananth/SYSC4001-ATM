#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "messageTypes.h"
#include "messageStructs.h"

void initializeDatabase();
void updateDatabase(my_message msg);
void createMessageQueues();
void DBServer();
void writeToDatabase();
my_message readFromDatabase();
int sendMessageToDBServer(my_message msg);
int withdrawFromAccount(my_message msg);
void blockAccount(my_message mg);
int searchDatabase(my_message msg);
void encodePIN(char originalPin[3],char encodedPin[3]);
void decodePIN(char originalPin[3],char decodedPin[3]);
int getSize();

//Database 
typedef struct Account{
	char accountNumber[6];
	char encodedPIN[4];
	float fundsAvailable;
	int attempts;
}Account;

Account database[3]; // The bank allows 5 accounts. It's a pretty small bank 

//Variables 
char * prevAccount;
int ATMServerMsgqid;
int ServerATMMsgqid;
int ServerEditorMsgqid;
//int trials = 0;

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
		int msgLength = sizeof(my_message) - sizeof(long);
		if(msgrcv(ATMServerMsgqid, &ATMMessage, msgLength, 0, IPC_NOWAIT) == -1){
			//If receiving failed print error and exit
		}
		else{	
			//switch case
			printf("Message Received\n");
			switch(ATMMessage.message_type){ //act acording to each case
				case 1: //pin message
					printf("PIN Message Received From ATM\n");
					int i = searchDatabase(ATMMessage);
					printf("database index: %d\n",i);
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
					}
					else{
						char decodedPin[4];
						decodePIN(database[i].encodedPIN,decodedPin);
						if(strcmp(ATMMessage.accountInfo.pin, decodedPin) == 0){
							database[i].attempts = 0;
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
						else{
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
							database[i].attempts +=1;
							if(database[i].attempts  == 3){
								blockAccount(ATMMessage);
							}
						}
					}
					break;
				case 4: //request funds
					printf("Request Funds Message Received From ATM\n");
					my_message fundsMessage;
					fundsMessage.message_type = getFunds;
					fundsMessage.accountInfo = ATMMessage.accountInfo;
					fundsMessage.funds = database[searchDatabase(ATMMessage)].fundsAvailable;
					printf("%f\n",fundsMessage.funds);
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
			}
		}
		
		//receive all from editor
		my_message UpdateMessage;
		int msgLength2 = sizeof(my_message) - sizeof(long);
		if(msgrcv(ServerEditorMsgqid, &UpdateMessage, msgLength2, 6, IPC_NOWAIT) == -1){
			//If receiving failed print error and exit
		}
		else{
			//if received, edit file
			printf("UpdateDB Message Received From Editor\n");
			updateDatabase(UpdateMessage);
		}
	}
}

//initialize the DB with the given accounts
void initializeDatabase(){
	Account account1, account2, account3;
	strcpy(account1.accountNumber,"00001");
	account1.accountNumber[5] = '\0';
	strcpy(account1.encodedPIN,"218");
	account1.fundsAvailable = (float)3443.22;
	account1.encodedPIN[3] = '\0';
	account1.attempts = 0;
	
	strcpy(account2.accountNumber,"00011");
	account2.accountNumber[5] = '\0';
	strcpy(account2.encodedPIN,"434");
	account2.fundsAvailable = 10089.97;
	account1.encodedPIN[3] = '\0';
	account2.attempts = 0;
	
	strcpy(account3.accountNumber,"00117");
	account3.accountNumber[5] = '\0';
	strcpy(account3.encodedPIN,"360");
	account3.fundsAvailable = 112.00;
	account1.encodedPIN[3] = '\0';
	account3.attempts = 0;
	
	database[0] = account1;
	database[1] = account2;
	database[2] = account3;
	writeToDatabase();
	
}

void updateDatabase(my_message msg){
	Account account;;
	strcpy(account.accountNumber,msg.accountInfo.accountNum);
	account.accountNumber[5] = '\0';
	strcpy(account.encodedPIN,msg.accountInfo.pin);
	account.fundsAvailable = (float)3443.22;
	account.encodedPIN[3] = '\0';
	account.attempts = 0;
	
	//allocate memory
	//add to database
	
	writeToDatabase();
	printf("Edited Database Successfully\n");
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
		fprintf(output, "%s\t%s\t%.2f\n",database[i].accountNumber, database[i].encodedPIN, database[i].fundsAvailable);
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
	
void decodePIN(char originalPin[3],char decodedPin[4]){
	
	int i;
	for(i = 0; i < 3; i++){
		if(originalPin[i] == '0'){
			decodedPin[i] = '9';
		}
		else{
			decodedPin[i] = originalPin[i] -1;
		} 
	}
	decodedPin[3] = '\0';
}

int getSize(){
	return (sizeof(database)/sizeof(Account));
}
