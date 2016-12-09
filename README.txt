* TEAM MEMBERS
	- Shasthra Ranasinghe (100867803)
	- Akhila Ananth	(100894838)

* LIST OF FILES

Part1.docx
ATM.c
DBEditor.c
DBServer.c
messageStructs.h
messageTypes.h
DB.txt
partB.c
partB_DB.txt


* SETUP INSTRUCTIONS

For part A:
The following commands are needed to run the three c files concurrently
note each command needs to be run in their own individual terminal.

gcc -o ATM.o ATM.c
./ATM

gcc -o DBEditor.o DbEditor.c
./DBEditor

gcc -o DBServer.o DBServer.c
./DBServer


For part B
The following commands are needed to run the three c files concurrently
note each command needs to be run in their own individual terminal.

gcc -o partB_ATM.o partB_ATM.c -pthread
./partB


* DESIGN CHOICES
	+ Part A
		- ATM, SERVER, and EDITOR are all seperate processes.
		- Each process can run concurrently
		- Server switches between listening to the ATM and EDITOR
		+ 3 Message Queues were used
			+ 2 queues between the ATM and SERVER
				- one queue is used to send message to the SERVER
					-The SERVER will listen to the above queue to get the ATM messages
				- one queue is used to listen on the SERVER
					- SERVER place messages on this queue for the ATM to take
			+ 1 queue between SERVER and EDITOR
				- Since messages only go one way, and there is only one type of message
					one one message queue was neccesarry
			- The reason for doing this was so the logic is easier to write
			- Both sides can listen to all messages on their own receive message
				and they will receive everything it needs and nothing more
			- Something that could have changed was using the same queue for the EDITOR and the ATM
				so instead of the SERVER alternating between ATM and EDITOR, it can just listen to the one queue
	+ Part B
		- The SERVER will be the main process that will spawn the ATM and EDITOR as a single thread
		- One implementation would be to have the ATM as a separate process. so that the SERVER and EDITOR
			will be in a second process. Reasons behind this is that it is possible to have multiple ATMs
			and the user should never have the option of editing the Database.
			The reason we went with one process was because in partB it was requested to change each process
			into a thread, and we assumed this was what was requested.

* TESTING
	+ Part A
		- Open the server, and then the atm and editor.
		- The editor and atm will request for account number
		+ ATM
			- Entering x or X on the ATMwill close it down
			- Once the account and pin is given in the atm the user will have the choice
				of withdrawing funds or requesting funds
			- Once the user gets a response for withdraw or request funds, it will go back to asking for a new user
		+ SERVER
			- The server runs forever 
		+ EDITOR
			- Entering x or X on the EDITOR will close it down 
			- The editor will as for the user information
			- if the account number of an existsing account is entered, it will edit the pin and funds of that account
				instead of making a new account and adding it to the DB
		+ Open the DB.txt file to see the state of the Database
			- Once the account is blocked you will see the the X in the file
			- when funds are withdrawn, the file will update aswell
			
	+ Part B
		- The user will initially have the option of choosing between the ATM and EDITOR
		+ If the EDITOR is chosen
			- The user will get to enter the information of one account and the it will go back to asking 
				to choose between the ATM and the EDITOR
		+ If the ATM is chosen
			- The user will be asked to enter account details just like in Part A.
			- Once one customer has completed a task, it will go back to asking to choose between
				ATM and EDITOR
		- The DB.txt is the same as in part A where we can check the file to see hwo the database is being updated.
