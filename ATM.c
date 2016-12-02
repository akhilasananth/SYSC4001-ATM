#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

int trialCount = 3;

typedef struct PINMessage{
	char accountNum[5];
	char pin[3];
	
}PINMessage;

int main (void){
	
	int key, mask, msgid;

	key = getuid();
	mask = 0666;
	msgid = msgget(key, mask);	
	if (msgid == -1) {
		msgid = msgget(key, mask | IPC_CREAT);
		if (msgid == -1) {
			fprintf(stderr, "Could not create message queue.\n");
			exit(EXIT_FAILURE);
		}
		
		
	
	while(1){
		//Get account information
		int accountNum = requestAccountNumber();
		int pinNum = requestPinNumber();
	
		//Create PIN Message and send to DB Server
		sendMessageToDBServer(createPINMessage(accountNumber, pinNumber));
	
		//Receive a "OK" or "NOT OKAY" message
		if(receiveMessageFromDBServer() == -1){
			//If receiving failed print error and go back to the start
			printf("Message Receive Failed\n");
			tiralCount = 3;
			continue;
		}else{
			//if the message is received check if its okay or not
			if(isOK(message){
				trialCount = 3;
				//repeat till terminated
				while(1){
					//request user for request funds or withdraw
					int response = requestUserForNextStep();
					if(response == 1{ //request Funds
						sendMessageToDBServer(createRequestFundsMessage("RequestFunds");
						if(receiveMessageFromDBServer() == -1){
							printf("Message Receive Failed\n");
							break;
						}else{
							printf("Available Funds: %d\n",message.Data);
						}
					}else if(response == 2){ //withdraw
						int funds = requestWithdrawAmount();
						sendMessageToDBServer(createWithdrawMessage("WithdrawFunds",funds);
						if(receiveMessageFromDBServer() == -1){
							printf("Message Receive Failed\n");
							break;
						}else{
							//check if enough funds
							if(isEnough(){
								printf("Enough Funds\n");
								break;
							}else{
								//if not enough start for a new customer
								printf("Not Enough Funds\n");
								trialCount = 3;
								int response = RequestContinue();
								if(response == 1){
									continue;
								}else{
									break;
								}
							}
						}
					}else{
						break;
					}
				}
				break;
			}else{
				//got a pin not okay message
				printf("PIN Not OK, Please try again\n");
				trialCount--;
			}
			
			//check if attempted 3 times
			if(tiralCount == 0){
				printf("Account is Blocked\n");
				sendMessageToDBServer(createMessage("AccountBlocked");
				trialCount = 3;
			}
		}
	}
	
	return 1;
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
		
		if(strlen(userInput) != maxSize){
			printf("Invalid entry\n");
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


	
	

}

int sendMessageToDBServer(createPINMessage(accountNumber, pinNumber)){
	
	
}

int receiveMessageFromDBServer(){
	
}

