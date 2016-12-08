#ifndef messageStructs
#define messageStructs


typedef struct PINMessage{
	char accountNum[6];
	char pin[4];
	
}PINMessage;

typedef struct my_message{
	long message_type;
	PINMessage accountInfo;
	float funds;
	float withdrawAmount;
}my_message;


#endif
