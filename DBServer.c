#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>

typedef struct accountInfo{
	char * accountNum;
	char * pin;
	
}accountInfo;

int main (void){
	
	accountInfo aInfo;
	char uAccountNum[100];
	char uPin[100];
	
	while(1){
		printf("Enter your 5 digit account number: ");
		scanf("%s", uAccountNum);
		if(strlen(uAccountNum) != 5){
			printf("Invalid entry\n");
			continue;
		}else{
			aInfo.accountNum = uAccountNum;
			printf("Enter your 3 digit account number: ");
			scanf("%s", uPin);
			if(strlen(uPin) != 3){
				printf("Invalid entry\n");
				continue;
			}else{
				aInfo.pin = uPin;
				break;
			}
		}
	}
	printf("Account Number: %s   Pin: %s",aInfo.accountNum,aInfo.pin);
	return 1;
}
