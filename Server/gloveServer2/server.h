#pragma once

#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <string>

class Server {
	 
	SOCKET sock;					// server' open socket,
	SOCKET clientSock;				// store client socket
	WSADATA wsa;					// Window Socket DATA
	struct sockaddr_in sockInfo, clientInfo;
	int clientSize;
public:
	Server();
	~Server();
	void start();					// start glove server
	int startup();					// create socket, bind, listen
	void printRibbon();				// print a pretty ribbon
	void printSSM();				// print fantastic SSM logo
	void listening();				// accept client here
	void CreateModelFile(int, void*);		// GRT
	int login(void*);				// check valid client
	
	int readTrainFromDB(int, void*);
	int receive(int, void*);
	void withClient(void*);			// do something with client
	
	std::thread runThread();		// run "withClient" func thread
	std::thread runGRT(int, void*);		// run "CreateModelFile" func thread
	std::thread runDBRefresh(int, void*);
	SOCKET returnClientSock();		// return clientSock

	bool isThisClassNew(int, int, void*);
};

// made this func into class Server
//unsigned int __stdcall withClient(void *data);

void fileReceive(SOCKET, char*);	// receive training from client
void fileSend(SOCKET, char*);		// send model to client
int sendData(SOCKET, char*);		// send data to client
int recvData(SOCKET, char*, int);	// receive data from client

