#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int trialCount = 3;

int main (void){
	
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
				tiralCount = 3;
				//repeat till terminated
				while(true){
					//request user for request funds or withdraw
					int response = requestUserForNextStep();
					if(response == 1{ //request Funds
						sendMessageToDBServer(createRequestFundsMessage("RequestFunds");
						if(receiveMessageFromDBServer() == -1){
							printf("Message Receive Failed\n");
							break;
						}else{
							printf("Avilable Funds: %d\n",message.Data);
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
							}else{
								//if not enough start for a new customer
								printf("Not Enough Funds\n");
								trialCount = 3;
								break;
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
