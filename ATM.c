#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "messageTypes.h"
#include "messageStructs.h"

void requestAccountInformation(PINMessage aInfo);
void requestCustomer(PINMessage aInfo, int maxSize);
void checkForExit(char * userInput);
void ATM(int ATMServerMsgqid);
int sendMessageToDBServer(int msgqid, my_message msg);
int receiveMessageFromDBServer(int msgqid, my_message msg);
int requestUserForNextStep();
int requestWithdrawAmount();

int main (void){
	
	//int key, mask, msgid;
	
	int ATMServerMsgqid = msgget((key_t)1234, IPC_CREAT| 0600);
	if(ATMServerMsgqid == -1){
		perror("msgget: msgget failed\n");
		exit(1);
	}
	ATM(ATMServerMsgqid);
	
	return 1;
}

void ATM(int ATMServerMsgqid){
	while(1){
		//Get account information
		PINMessage aInfo;
		requestAccountInformation(aInfo);
	
		//Create PIN Message and send to DB Server
		my_message msg;
		msg.message_type = 1;
		msg.accountInfo = aInfo;
		
		if(sendMessageToDBServer(ATMServerMsgqid, msg) == -1){
			perror("msgsnd: msgsnd failed\n");
		}
	
		//Receive a "OK" or "NOT OKAY" message
		my_message okNotOkaymsg;
		if(receiveMessageFromDBServer(ATMServerMsgqid,okNotOkaymsg) == -1){
			//If receiving failed print error and go back to the start
			perror("msgrcv: msgrcv failed\n");
			continue;
		}else{
			//if the message is received check if its okay or not
			if(1){
				//repeat till terminated
				while(1){
					//request user for request funds or withdraw
					int response = requestUserForNextStep();
					if(response == 1){ //request Funds
						sendMessageToDBServer(ATMServerMsgqid, msg);
						if(receiveMessageFromDBServer(ATMServerMsgqid, msg) == -1){
							printf("Message Receive Failed\n");
							break;
						}else{
							printf("Available Funds: %d\n",100);
						}
					}else if(response == 2){ //withdraw
						int funds = requestWithdrawAmount();
						sendMessageToDBServer(ATMServerMsgqid, msg);
						msg.funds = funds;
						if(receiveMessageFromDBServer(ATMServerMsgqid, msg) == -1){
							printf("Message Receive Failed\n");
							break;
						}else{
							//check if enough funds
							if(1){
								printf("Enough Funds\n");
								break;
							}else{
								//if not enough start for a new customer
								perror("Not Enough Funds\n");
							}
						}
					}else{
						break;
					}
				}
				break;
			}else{
				//got a pin not okay message
				perror("PIN Not OK, Please try again\n");
			}
			
			//check if attempted 3 times
			//if(tiralCount == 0){
			//	printf("Account is Blocked\n");
			//	sendMessageToDBServer(createMessage("AccountBlocked");
			//}
		}
	}
}

//This method requests account number and pin from the customer
void requestAccountInformation(PINMessage aInfo){
	
	//Requests an account number
	requestCustomer(aInfo,5);
	
	//Requests a PIN
	requestCustomer(aInfo,3);
}

//Here if it requests the account number then the maxSize is 5 and 
//if it requests the pin, then the maxSize is 3
void requestCustomer(PINMessage aInfo, int maxSize){
	char userInput[100];
	while(1){
		printf("Enter your %d digit account number: ",maxSize);
		scanf("%s", userInput);
		checkForExit(userInput);
		
		if(strlen(userInput) != maxSize){
			perror("Invalid entry\n");
			continue;
		}
		
		else{
			if(maxSize == 5){
				aInfo.accountNum = userInput;
			}
			else{
				aInfo.pin = userInput;
			}
			break;
		}
	}
	
}

void checkForExit(char * userInput){
	if(strcmp(userInput,"X")==0){
		printf("Closing ATM\n");
		exit(1);
	}
}


int sendMessageToDBServer(int msgqid, my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgsnd(msgqid, &msg, msgLength,0);
}

int receiveMessageFromDBServer(int msgqid, my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgrcv(msgqid, &msg, msgLength, 0, 0);
}

int requestUserForNextStep(){
	return 1;
}

int requestWithdrawAmount(){
	return 100;
}
