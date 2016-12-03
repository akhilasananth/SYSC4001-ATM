#ifndef messageStructs
#define messageStructs


typedef struct PINMessage{
	char * accountNum;
	char * pin;
	
}PINMessage;

typedef struct my_message{
	long message_type;
	PINMessage accountInfo;
	double funds;
}my_message;


#endif
