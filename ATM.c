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
PINMessage requestCustomer();
void checkForExit(char * userInput);
int sendMessageToDBServer(my_message msg);
int requestUserForNextStep();
int requestWithdrawAmount();
void printPin(char pin[3]);


//Variables
int ATMServerMsgqid;
int ServerATMMsgqid;

int main (void){

	printf("Starting ATM...\nType \"X\" to exit\n");
	createMessageQueues();
	ATM();
	
	return 1;
}

void ATM(){
	while(1){
		//Get account information
		PINMessage aInfo;
		aInfo = requestCustomer();

		//Create PIN Message and send to DB Server
		my_message msg;
		msg.message_type = pinMessage;
		strcpy(msg.accountInfo.accountNum,aInfo.accountNum);
		strcpy(msg.accountInfo.pin,aInfo.pin);
		
		if(sendMessageToDBServer(msg) == -1){
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
				int response = requestUserForNextStep();
				if(response == 1){ //request Funds
					fundsMsg.message_type = requestFunds;
					strcpy(fundsMsg.accountInfo.accountNum,aInfo.accountNum);
					strcpy(fundsMsg.accountInfo.pin,aInfo.pin);
					
				}
				else if(response == 2){ //withdraw
					fundsMsg.message_type = withdraw;
					strcpy(fundsMsg.accountInfo.accountNum,aInfo.accountNum);
					strcpy(fundsMsg.accountInfo.pin,aInfo.pin);
					fundsMsg.withdrawAmount = requestWithdrawAmount();
				}
				//Send message to server
				if(sendMessageToDBServer(fundsMsg) == -1){
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
	}
}


//Here if it requests the account number then the maxSize is 5 and 
//if it requests the pin, then the maxSize is 3
PINMessage requestCustomer(){
	PINMessage customerInput;
	while(1){
		
		printf("Enter your 5 digit account number: ");
		scanf("%s", customerInput.accountNum);
		customerInput.accountNum[5] = '\0';
		checkForExit(customerInput.accountNum);
		
		if(strlen(customerInput.accountNum) != 5){
			perror("Invalid entry\n");
			continue;
		}

		printf("Enter your 3 digit PIN number: ");
		scanf("%s", customerInput.pin);
		customerInput.pin[3] = '\0';

		checkForExit(customerInput.pin);

		while(strlen(customerInput.pin) != 3){
			perror("Invalid entry\n");
			printf("Enter your 3 digit PIN number: ");
			scanf("%s", customerInput.pin);
			customerInput.pin[3] = '\0';

			checkForExit(customerInput.pin);
			
		}
		break;

	}
	return customerInput;
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
	if(strcmp(userInput,"X")==0 || strcmp(userInput,"x")==0 ){
		printf("Closing ATM\n");
		exit(1);
	}
}


int sendMessageToDBServer(my_message msg){
	int msgLength = sizeof(my_message) - sizeof(long);
	return msgsnd(ATMServerMsgqid, &msg, msgLength,0);
}

int requestUserForNextStep(){
	int userInput;
	while(1){
		printf("\nEnter the number corresponding to your choice:\n");
		printf("(1) Request Funds\n");
		printf("(2) Withdraw\n");
		scanf("%d", &userInput);
		
		if(userInput == 1){ //Withdraw message
			printf("Requesting Funds...\n");
			break;
		}
		else if(userInput == 2){ //Request Funds
			printf("\nWithrdawing Funds...\n");
			break;
		}
		else{
			perror("Invalid Choice.\n");
		}
	}
	return userInput;
}

int requestWithdrawAmount(){
	float userInput;
	printf("\nEnter Withdraw Amount: ");
	scanf("%f", &userInput);
	return userInput;
}

void printPin(char pin[3]){
	int i;
	for(i = 0; i<3; i++){
		printf("%c \n",pin[i]);
	}
}
