#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "messageTypes.h"
#include "messageStructs.h"

void createMessageQueues();
void ATM();
void requestAccountInformation(PINMessage aInfo);
void requestCustomer(PINMessage aInfo, int maxSize);
void checkForExit(char * userInput);
int sendMessageToDBServer(my_message msg);
int receiveMessageFromDBServer(my_message msg);
int requestUserForNextStep();
int requestWithdrawAmount();

int attempts = 0;
char * prevAccount;
int ATMServerMsgqid;
int ServerATMMsgqid;

int main (void){

	createMessageQueues();
	ATM();
	
	return 1;
}

void ATM(){
	while(1){
		printf("Starting ATM...\nType \"X\" to exit\n");
		//Get account information
		PINMessage aInfo;
		requestAccountInformation(aInfo);
	
		//Create PIN Message and send to DB Server
		my_message msg;
		msg.message_type = pinMessage;
		msg.accountInfo = aInfo;
		msg.funds = 12345;
		
		if(sendMessageToDBServer(msg) == -1){
			perror("msgsnd: msgsnd failed\n");
			exit(1);
		}
		else{
			printf("msgsnd: msgsnd to ServerDB sucess\n");
		}
	
		//Receive a "OK" or "NOT OKAY" message
		my_message okNotOkaymsg;
		printf("Waiting for Response from Server...\n");
		if(receiveMessageFromDBServer(okNotOkaymsg) == -1){
			//If receiving failed print error and exit
			perror("msgrcv: msgrcv failed\n");
			exit(1);
		}
		else{
			printf("msgrcv: msgrcv from ServerDB sucess\n");
			//if the message is ok
			if(okNotOkaymsg.message_type == ok){
				printf("PIN number OK!");
				
				my_message fundsMsg;
				//request user for request funds or withdraw
				int response = requestUserForNextStep();
				if(response == 1){ //request Funds
					fundsMsg.message_type = requestFunds;
					fundsMsg.accountInfo = aInfo;
					
				}
				else if(response == 2){ //withdraw
					fundsMsg.message_type = withdraw;
					fundsMsg.accountInfo = aInfo;
					fundsMsg.withdrawAmount = requestWithdrawAmount();
				}
				//Send message to server
				if(sendMessageToDBServer(fundsMsg) == -1){
					perror("msgsnd: msgsnd failed\n");
					exit(1);
				}
				else{
					printf("msgsnd: msgsnd to ServerDB sucess\n");
				}
				
				my_message fundsResponseMsg;
				if(receiveMessageFromDBServer(fundsResponseMsg) == -1){
					//If receiving failed print error and exit
					perror("msgrcv: msgrcv failed\n");
					exit(1);
				}
				else{
					printf("msgrcv: msgrcv from ServerDB sucess\n");
					
					if(fundsResponseMsg.message_type == getFunds){ //funds
						printf("Available Funds: %f\n",fundsResponseMsg.funds);
					}else if(fundsResponseMsg.message_type == notEnoughFunds){ //not enough funds
						perror("Not Enough Funds\n");
					}else if(fundsResponseMsg.message_type == enoughFunds){ //enough funds
						printf("Enough Funds\n");
					}
					
				}
			}
			else if(okNotOkaymsg.message_type == notOk){
				//got a pin not okay message
				perror("PIN Not OK, Please try again\n");
				if(strcmp(prevAccount,okNotOkaymsg.accountInfo.accountNum)==0){
					attempts = attempts +1;
					if(attempts == 3){
						printf("PIN Not OK, Account : %s Blocked!\n",prevAccount);
						attempts = 0;
						break;
					}
				}
				else{
					prevAccount = okNotOkaymsg.accountInfo.accountNum;
				}
			}
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
		if(maxSize == 5){
			printf("Enter your %d digit account number: ",maxSize);
		}
		else{
			printf("Enter your %d digit PIN number: ",maxSize);
		}
		scanf("%s", userInput);
		checkForExit(userInput);
		
		if(strlen(userInput) != maxSize){
			perror("Invalid entry\n");
			continue;
		}
		
		else{
			if(maxSize == 5){
				strcpy(aInfo.accountNum,userInput);
				prevAccount = userInput;
			}
			else{
				strcpy(aInfo.pin,userInput);
			}
			break;
		}
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
}

void checkForExit(char * userInput){
	if(strcmp(userInput,"X")==0){
		printf("Closing ATM\n");
		exit(1);
	}
}


int sendMessageToDBServer(my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgsnd(ATMServerMsgqid, &msg, msgLength,0);
}

int receiveMessageFromDBServer(my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgrcv(ServerATMMsgqid, &msg, msgLength, 0, 0);
}

//need fixing
int requestUserForNextStep(){
	int userInput;
	while(1){
		printf("\nEnter the number corresponding to your choice:\n");
		printf("(1) Request Funds\n");
		printf("(2) Withdraw\n");
		scanf("%d", &userInput);
		//checkForExit(userInput);
		
		if(userInput == 1){ //Withdraw message
			printf("\nRequest Funds:\n");
			break;
		}
		else if(userInput == 2){ //Request Funds
			printf("\nWithrdaw:\n");
			break;
		}
		else{
			perror("Invalid Choice.\n");
		}
	}
	return userInput;
}

//needs fixing
int requestWithdrawAmount(){
	float userInput;
	printf("\nEnter Withdraw Amount: ");
	scanf("%f", &userInput);
	//checkForExit(userInput);
	return userInput;
}
