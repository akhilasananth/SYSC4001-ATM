#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "messageTypes.h"
#include "messageStructs.h"
#include <pthread.h>
#include <ctype.h>

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
void encodePIN(char originalPin[4],char encodedPin[3]);
void decodePIN(char originalPin[4],char decodedPin[3]);
int getSize();
void *userInputMethod(void *arg);


//Database 
typedef struct Account{
	char accountNumber[6];
	char encodedPIN[4];
	float fundsAvailable;
	int attempts;
}Account;

Account database[50]; // The bank allows 5 accounts. It's a pretty small bank 

//Variables 
int ATMServerMsgqid;
int ServerATMMsgqid;
int ServerEditorMsgqid;
pthread_t userInput;

int main (void){
	createMessageQueues();
	initializeDatabase();
	
	if(pthread_create(&userInput, NULL, userInputMethod, 0)){
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}
	
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
			my_message sendMsg;
			int i;
			//switch case
			switch(ATMMessage.message_type){ //act acording to each case
				case 1: //pin message
					i = searchDatabase(ATMMessage);
					if(i == -1){
						sendMsg.message_type = notOk;
						sendMsg.accountInfo = ATMMessage.accountInfo;
						if(sendMessageToDBServer(sendMsg) == -1){
							perror("msgsnd: msgsnd to ATM failed\n");
							exit(1);
						}
					}
					else{
						char decodedPin[4];
						decodePIN(database[i].encodedPIN,decodedPin);
						if(strcmp(ATMMessage.accountInfo.pin, decodedPin) == 0){
							database[i].attempts = 0;
							sendMsg.message_type = ok;
							sendMsg.accountInfo = ATMMessage.accountInfo;
							if(sendMessageToDBServer(sendMsg) == -1){
								perror("msgsnd: msgsnd to ATM failed\n");
								exit(1);
							}
							continue;
						}
						else{
							sendMsg.message_type = notOk;
							sendMsg.accountInfo = ATMMessage.accountInfo;
							if(sendMessageToDBServer(sendMsg) == -1){
								perror("msgsnd: msgsnd to ATM failed\n");
								exit(1);
							}
							database[i].attempts +=1;
							if(database[i].attempts  == 3){
								blockAccount(ATMMessage);
							}
						}
					}
					break;
				case 4: //request funds
					sendMsg.message_type = getFunds;
					sendMsg.accountInfo = ATMMessage.accountInfo;
					sendMsg.funds = database[searchDatabase(ATMMessage)].fundsAvailable;
					if(sendMessageToDBServer(sendMsg) == -1){
						perror("msgsnd: msgsnd to ATM failed\n");
						exit(1);
					}
					break;
				case 5: //withdraw
					i = withdrawFromAccount(ATMMessage);
					sendMsg.message_type = i;
					sendMsg.accountInfo = ATMMessage.accountInfo;
					
					if(sendMessageToDBServer(sendMsg) == -1){
							perror("msgsnd: msgsnd to ATM failed\n");
							exit(1);
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
	int i = searchDatabase(msg);
	if(i == -1){
		Account account;
		strcpy(account.accountNumber,msg.accountInfo.accountNum);
		account.accountNumber[5] = '\0';
		strcpy(account.encodedPIN,msg.accountInfo.pin);
		account.fundsAvailable = (float)msg.funds;
		account.encodedPIN[3] = '\0';
		account.attempts = 0;
		char encodedPin[4];
		encodePIN(account.encodedPIN,encodedPin);
		strcpy(account.encodedPIN,encodedPin);
		
		//add to database
		database[getSize()] = account;
		
		writeToDatabase();
	}
	else{
		strcpy(database[i].encodedPIN,msg.accountInfo.pin);
		database[i].fundsAvailable = (float)msg.funds;
		database[i].encodedPIN[3] = '\0';
		database[i].attempts = 0;
		char encodedPin[4];
		encodePIN(database[i].encodedPIN,encodedPin);
		strcpy(database[i].encodedPIN,encodedPin);
		
		writeToDatabase();
	}
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
		perror("error: could not open file");
		exit(0);
	}
	int i;
	for(i = 0; i<getSize();++i){
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
	for(i = 0; i < getSize(); i++){
		if(strcmp(msg.accountInfo.accountNum,database[i].accountNumber) == 0){
			return i;
		}
	}
	return -1;
}

void encodePIN(char originalPin[4],char encodedPin[3]){
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
	
void decodePIN(char originalPin[4],char decodedPin[4]){
	
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
	int i;
	for(i = 0;i<(sizeof(database)/sizeof(Account));i++){
		if(strlen(database[i].accountNumber) < 5){
			break;
		}
	}
	return i;
}


void *userInputMethod(void *arg){
	int input;
	while(1){
		printf("Choose one of the options:\n");
		printf("(1) ATM\n");
		printf("(2) DBEditor\n");
		printf("(3) EXIT\n");
		scanf("%d", &input);
		
		
//ATM :
		if(input == 1){
			printf("\nStarting ATM...\nType \"X\" to exit\n");

			//Get account information
			PINMessage aInfo;
			
			//Here account number and pin number is requested from the customer 
			while(1){
				printf("Enter your 5 digit account number: ");
				scanf("%s", aInfo.accountNum);

				if(strcmp(aInfo.accountNum,"X")==0 || strcmp(aInfo.accountNum,"x")==0 ){
					printf("Closing ATM\n");
					exit(1);
				}
				
				if(strlen(aInfo.accountNum) != 5){
					perror("Invalid entry\n");
					continue;
				}
				aInfo.accountNum[5] = '\0';

				printf("Enter your 3 digit PIN number: ");
				scanf("%s", aInfo.pin);
				aInfo.pin[3] = '\0';

				if(strcmp(aInfo.pin,"X")==0 || strcmp(aInfo.pin,"x")==0 ){
					printf("Closing ATM\n");
					exit(1);
				}

				while(strlen(aInfo.pin) != 3){
					perror("Invalid entry\n");
					printf("Enter your 3 digit PIN number: ");
					scanf("%s", aInfo.pin);
					aInfo.pin[3] = '\0';

					if(strcmp(aInfo.pin,"X")==0 || strcmp(aInfo.pin,"x")==0 ){
					printf("Closing ATM\n");
					exit(1);
					}
				}
				break;
			}

			//Create PIN Message and send to DB Server
			my_message msg;
			msg.message_type = pinMessage;
			strcpy(msg.accountInfo.accountNum,aInfo.accountNum);
			strcpy(msg.accountInfo.pin,aInfo.pin);
			
			if(msgsnd(ATMServerMsgqid, &msg, (sizeof(my_message) - sizeof(long)),0) == -1){
				perror("msgsnd: msgsnd failed\n");
				exit(1);
			}
			else{
				//printf("msgsnd: msgsnd to ServerDB sucess\n");
			}
		
			//Receive a "OK" or "NOT OKAY" message
			my_message okNotOkaymsg;
			printf("Waiting for Response from Server...\n");
			int msgLength = sizeof(my_message) - sizeof(long);
			if(msgrcv(ServerATMMsgqid, &okNotOkaymsg, msgLength, 0, 0) == -1){
				//If receiving failed print error and exit
				perror("msgrcv: msgrcv failed\n");
				exit(1);
			}
			else{
				//printf("msgrcv: msgrcv from ServerDB sucess\n");
				//if the message is ok
				if(okNotOkaymsg.message_type == ok){
					printf("PIN number OK!");
					
					my_message fundsMsg;
					//request user for request funds or withdraw
					
					//Request User for the next step
					int response;
					while(1){
						printf("\nEnter the number corresponding to your choice:\n");
						printf("(1) Request Funds\n");
						printf("(2) Withdraw\n");
						scanf("%d", &response);
		
						if(response == 1){ //Withdraw message
							printf("Requesting Funds...\n");
							break;
						}
						else if(response == 2){ //Request Funds
							printf("\nWithrdawing Funds...\n");
							break;
						}
						else{
							perror("Invalid Choice.\n");
						}
					}
					
					if(response == 1){ //request Funds
						fundsMsg.message_type = requestFunds;
						strcpy(fundsMsg.accountInfo.accountNum,aInfo.accountNum);
						strcpy(fundsMsg.accountInfo.pin,aInfo.pin);
						
					}
					else if(response == 2){ //withdraw
						fundsMsg.message_type = withdraw;
						strcpy(fundsMsg.accountInfo.accountNum,aInfo.accountNum);
						strcpy(fundsMsg.accountInfo.pin,aInfo.pin);
						
					//Request ATM withdraw amount
						printf("\nEnter Withdraw Amount: ");
						scanf("%f", &fundsMsg.withdrawAmount);
					}
					//Send message to server
					if(msgsnd(ATMServerMsgqid, &fundsMsg, (sizeof(my_message) - sizeof(long)),0) == -1){
						perror("msgsnd: msgsnd failed\n");
						exit(1);
					}
					else{
						//printf("msgsnd: msgsnd to ServerDB sucess\n");
					}
					
					my_message fundsResponseMsg;
					int msgLength2 = sizeof(my_message) - sizeof(long);
					if(msgrcv(ServerATMMsgqid, &fundsResponseMsg, msgLength2, 0, 0) == -1){
						//If receiving failed print error and exit
						perror("msgrcv: msgrcv failed\n");
						exit(1);
					}
					else{
						//printf("msgrcv: msgrcv from ServerDB sucess\n");
						
						if(fundsResponseMsg.message_type == getFunds){ //funds
							printf("Available Funds: %.2f\n",fundsResponseMsg.funds);
						}else if(fundsResponseMsg.message_type == notEnoughFunds){ //not enough funds
							perror("Not Enough Funds\n");
						}else if(fundsResponseMsg.message_type == enoughFunds){ //enough funds
							printf("Enough Funds\n");
						}
						
					}
				}
				else if(okNotOkaymsg.message_type == notOk){
					//got a pin not okay message
					perror("PIN Not OK\n");
				}
			}
			printf("Closing ATM...\n\n");
		}
		
//DB Editor:
		
		else if(input == 2){
			//Variables
			int iANum;
			int retANum = 1;
			
	
			printf("\nStarting DBEditor...\n Type \"X\" to exit\n");
			ServerEditorMsgqid = msgget((key_t)123456, IPC_CREAT| 0600);
	
			if(ServerEditorMsgqid == -1){
				perror("msgget: ATMServerMsgqid failed\n");
				exit(1);
			}
			//Get account information
			my_message editorInput;
			while(1){
				
				printf("Enter your 5 digit account number: ");
				scanf("%s", editorInput.accountInfo.accountNum);
			
				if(strcmp(editorInput.accountInfo.accountNum,"X")==0 || strcmp(editorInput.accountInfo.accountNum,"x")==0 ){
					printf("Closing ATM\n");
					exit(1);
				}
				
				
				for(iANum = 0; iANum< 5; iANum++){
					retANum = isdigit(editorInput.accountInfo.accountNum[iANum]);
					if(retANum == 0){
						break;
						}
				}
				
				if((strlen(editorInput.accountInfo.accountNum) != 5) || retANum == 0){
					perror("Invalid entry\n");
					continue;
				}
				
				editorInput.accountInfo.accountNum[5] = '\0';
				
				int iPin;
				int retPin = 1;
				
				do{
					printf("Enter your 3 digit PIN number: ");
					scanf("%s", editorInput.accountInfo.pin);
					
					for(iPin = 0; iPin< 3; iPin++){
					retPin = isdigit(editorInput.accountInfo.pin[iPin]);
						if(retPin == 0){
							break;
							}
					}

					if(strcmp(editorInput.accountInfo.pin,"X")==0 || strcmp(editorInput.accountInfo.pin,"x")==0 ){
						printf("Closing ATM\n");
						exit(1);
						}
					
					if(strlen(editorInput.accountInfo.pin) != 3){
						printf("Invalid Entry! \n");
					}
						
				}while(strlen(editorInput.accountInfo.pin) != 3 || retPin == 0);
				
				editorInput.accountInfo.pin[3] = '\0';

				do{ 
					printf("Enter funds available: ");
					scanf("%f", &editorInput.funds);
				}while(editorInput.funds< 0); //Checks for negative money
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
			printf("Closing DB Editor...\n\n");
		}
		else if(input == 3){
			printf("Closing ATM\n");
			exit(1);
		}
		else{
			perror("Invalid entry\n");
		}
	}
}

	


