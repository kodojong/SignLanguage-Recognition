#include "server.h"
#include "global.h"					// *.lib, serverInfo
#include "connectSQL.h"				// Database
#include "TrainingDataParser.h"		// *.grt parse
#include "Dyslexic.h"				// GRT
#include <iostream>

std::mutex iomutex;

std::string replaceAll(const std::string &str, const std::string &pattern, const std::string &replace);

/* @ ====================
* netData
* ====================*/
/*
* struct : {ConnectSQL* , SOCKET}
*
* Decription :
*  structure of mysql and socket
*  By packing this two in one structure,
*  we can hand over in one parameter(=(void*))
*/
struct netData {
	ConnectSQL sql;			/* store mysql data */
	SOCKET client;			/* store client socket data */
};

Server::Server() {
	clientSize = sizeof(clientInfo);
}

Server::~Server() {
	closesocket(sock);
	WSACleanup();
}

/* @ ====================
* Server::start()
* ====================*/
/*
* Function : void start()
*
* Decription :
*  run initiation 'startup'
*  print fantastic ssm logo
*  start listening
*
* Returns :
*  void
*/
void Server::start() {
	if (startup()) {
		std::cerr << "Server Start Fail\n";
		return;
	}

	//printRibbon();
	printSSM();

	std::cout << "Server Start!\n";
	listening();
}

/* @ ====================
* Server::startup()
* ====================*/
/*
* Function : int startup()
*
* Decription :
*  init winsock, socket
*
* Returns :
*  0, successful startup
*  1, when something goes wrong
*/
int Server::startup() {

	// Init WinSock Library ver2.2
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		std::cerr << "WSA startup fail\n";
		return 1;
	}

	// Create TCP Server Socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Invalid socket\n";
		return 1;
	}

	// edit socket infomation
	sockInfo.sin_family = AF_INET;
	sockInfo.sin_port = htons(atoi(GLOVE_PORT));
	sockInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// bind Socket
	if (bind(sock, (SOCKADDR *)&sockInfo, sizeof(sockInfo)) == SOCKET_ERROR) {
		std::cerr << "bind fail " << GetLastError() << "\n";
		return 1;
	}

	// server now listen to clients
	if (listen(sock, SOMAXCONN)) {
		std::cerr << "listen fail " << GetLastError() << "\n";
		return 1;
	}

	return 0;
}

/* @ ====================
* Server::listening()
* ====================*/
/*
* Function : void listening()
*
* Decription :
*  server listens to client
*  whenever a client accepts, create thread running 'withClient()'
*
* Returns :
*  void
*/
void Server::listening() {

	// accept forever(?)
	while (clientSock = accept(sock, (SOCKADDR *)&clientInfo, &clientSize)) {
		if (clientSock == INVALID_SOCKET) {
			std::cerr << "invalid client socket " << GetLastError() << "\n";
			continue;
		}

		// for safety, surround thread with { }
		{
			// run WithClient with other thread
			std::thread tmpThread = runThread();
			tmpThread.detach();
		}
	}
}

/* @ ====================
 * sendData()
 * ====================*/
/*
 * Function : int sendData(SOCKET, char*)
 *
 * Decription :
 *  Send data(=buffer) to connected client
 *
 * Returns :
 *  result of send()
 */
int sendData(
	SOCKET clientSock,		/* Client socket  */
	char *sendbuf			/* Buffer to send */
	) {
	return send(clientSock, sendbuf, strlen(sendbuf), 0);
}

/* @ ====================
 * recvData()
 * ====================*/
/*
 * Function : int sendData(SOCKET, char*, int)
 *
 * Decription :
 *  Receive data(=buffer) from connected client
 *
 * Returns :
 *  result of recv()
 */
int recvData(
	SOCKET clientSock,		/* Client socket */
	char *recvbuf,			/* store received buffer here */
	int size				/* length of how much to receive */
	) {
	int sz = recv(clientSock, recvbuf, size, 0);
	recvbuf[sz] = '\0';
	return sz;
}


/* @ ====================
 * fileReceive()
 * ====================*/
/*
 * Function : int fileReceive(SOCKET, char*)
 *
 * Decription :
 *  Receive training.grt from connected client
 *
 *  receive buffer in lines,
 *  loop(
 * * get line length in 'rec'
 * * get line buffer in 'buffer'
 * * sprintf 'buffer' in 'buffer2'
 * * write 'buffer2' in file
 *  )loop end
 *
 * Returns :
 *  void
 */
void fileReceive(
	SOCKET CLI,				/* Client socket */
	char* fileName			/* store received file with this name */
	) {

	char rec[1024];		/* receive buffer line length here */
	memset(rec, 0, 1024);
	//recvData(CLI, fileName, 32);
	//sendData(CLI, "OK\n");

	//FILE *fw;
	//fopen_s(&fw, fileName, "wb");
	//fopen_s(&fw, "fromAndroid1.grt", "wb");

	std::ofstream fw;
	fw.open(fileName, std::ios::out);

	//sendData(CLI, "OK\n");

	while (true) {
		int recs = recvData(CLI, rec, 4);
		
		rec[recs] = '\0';
		int size = atoi(rec);
		if (!size)break;
		char buffer[1030];
		char buffer2[1030];
		memset(buffer, 0, 1030);
		memset(buffer2, 0, 1030);
		if (size >= 1024) {
			recvData(CLI, buffer, 1024);
			//std::cout << buffer << std::endl;
			//sendData(CLI, "OK");
			//fwrite(buffer, 1024, 1, fw);

			sprintf_s(buffer2, "%s\n", buffer);
			fw.write(buffer2, size + 1);
		}
		else {
			recvData(CLI, buffer, size);
			buffer[size] = '\n';
			//std::cout << buffer << std::endl;
			//sendData(CLI, "OK");
			//fwrite(buffer, size, 1, fw);

			sprintf_s(buffer2, "%s\n", buffer);
			fw.write(buffer2, size + 1);
		}
	}
	std::cout << " * received *\n";
	//fclose(fw);

	fw.close();
	sendData(CLI, "OK\n");
}

/* @ ====================
 * fileSend()
 * ====================*/
/*
 * Function : int fileSend(SOCKET, char*)
 *
 * Decription :
 *  Send model.grt to connected client
 *
 *  open file to send
 *  calc file size
 *  loop(
 * * read file buffer in 'buffer'
 * * send 'buffer' to client
 * * calc left size
 *  )loop until left size is zero
 *
 * Returns :
 *  void
 */
void fileSend(
	SOCKET CLI,				/* client socket */
	char *fpath				/* file path&name to send */
	) {

	
	char filename[50];
	int i = strlen(fpath);
	for (; i > 0; i--) { 
		if (fpath[i - 1] == '\\')
			break; 
	}
	
	for (int j = 0; i <= (int)strlen(fpath); i++)
		filename[j++] = fpath[i];

	std::ifstream myFile(fpath, std::ios::in | std::ios::binary | std::ios::ate);
	if (!myFile.good()) {
		sendData(CLI, "NO\n");
		return;
	}
	int size = (int)myFile.tellg();
	myFile.close();

	char filesize[10];//itoa(size, filesize, 10);
					  //_itoa(size, filesize, 10);
	_itoa_s(size, filesize, 10);
	//sendData(CLI, filename);
	char rec[32] = "";
	//recvData(CLI, rec, 32);

	//sendData(CLI, filesize);
	//recvData(CLI, rec, 32);
	std::cout << "start File Send\n";
	FILE *fr; // = fopen(fpath, "rb");
	!fopen_s(&fr, fpath, "rb");
	while (size > 0) {
		char buffer[1030];

		if (size >= 1024) {
			fread(buffer, 1024, 1, fr);
			send(CLI, buffer, 1024, 0);
//			recvData(CLI, rec, 32);

		}
		else {
			fread(buffer, size, 1, fr);
			buffer[size] = '\0';
			send(CLI, buffer, size, 0);
//			recvData(CLI, rec, 32);
		}


		size -= 1024;

	}

	sendData(CLI, "OK\n");
	std::cout << "done sending " << filename << std::endl;
	fclose(fr);
}

//
//unsigned int __stdcall withClient(void *data) {
//	ConnectSQL cSQL(data);
//	SOCKET *cl = (SOCKET*)data;
//	SOCKET CLI = *cl;
//	if (cSQL.connectMYSQL()) return 1;
//
//	{
//		std::lock_guard<std::mutex> iolock(iomutex);
//		std::cout << "Client connected, thread #" <<
//			GetCurrentThreadId() << " is running\n";
//	}
//
//
//	char buf1[1024];
//	memset(buf1, 0, sizeof(buf1));
//
//	recv(CLI, (char*)&buf1, sizeof(buf1), 0);
//	std::cout << buf1 << std::endl;
//
//	ProtocolBufferType1 buf1;
//	memset((char*)&buf1, 0, sizeof(buf1));
//	
//	recv(CLI, (char*)&buf1, sizeof(buf1), 0);
//	std::cout << buf1.A << " " << buf1.B << " " << buf1.C << std::endl;
//	if (buf1.A == '0') {
//		if (cSQL.LoginJoin(buf1.B, buf1.C)) {
//			{
//				std::lock_guard<std::mutex> iolock(iomutex);
//				std::cout << "login Fail, thread #" <<
//					GetCurrentThreadId() << " is closing\n";
//			}
//			return 1;
//		}
//		std::cout << "Login Success" << std::endl;
//	}// login process
//	else if (buf1.A == '1') {
//	}// time compare process
//	else {
//	}// wrong input
//
//	recvData
//	while (true) {
//
//		char rec[50];
//		memset(rec, 0, sizeof(rec));
//		int sz = recv(CLI, rec, 32, 0);
//		send(CLI, "OK", strlen("OK"), 0);
//
//		if (strcmp(rec, "FileSend") == 0) {
//			char fname[32];
//			memset(fname, 0, sizeof(fname));
//			fileReceive(CLI, fname);
//		}
//
//		if (strcmp(rec, "EndConnection") == 0) {
//			std::cout << "connection end" << std::endl;
//			break;
//		}
//	}
//
//	sendData
//	{
//		char rec2[32];
//		sendData(CLI, "FileSend");
//		recvData(CLI, rec2, 32);
//		fileSend(CLI, "trainingData1.grt");
//		std::cout << "file send\n";
//		sendData(CLI, "EndConnection");
//		recvData(CLI, rec2, 32);
//	}
//	closesocket(CLI);
//	TrainingDataParser parser;
//	parser.Start("trainingData1.grt", 3);
//
//	std::this_thread::sleep_for(std::chrono::milliseconds(900));
//	cSQL.sendBack();
//	{
//		std::lock_guard<std::mutex> iolock(iomutex);
//		std::cout << "Client thread #" <<
//			GetCurrentThreadId() << " disconnected\n";
//	}
//	return 0;
//}

int Server::readTrainFromDB(int userid, void* data) {
	
	// client, cSQL init
	ConnectSQL tmpCSQL = ((netData*)data)->sql;
	SOCKET client = ((netData*)data)->client;

	// count NumberOfClasses
	std::string query = "select count(distinct classid) from wordtable where userid = '";
	query.append(std::to_string(userid));
	query += ("'");

	if (tmpCSQL.queryMYSQL(query.c_str()))return -1;
	int numClass = tmpCSQL.resultFirstLineInINT();
	int numDimen = 0, totalNumTrainingExamples;
	std::vector<GRT::ClassTracker> classTracker;

	for (int kkk = 0;kkk < 2;kkk++) {
		totalNumTrainingExamples = 0;
		classTracker.clear();
		classTracker.resize(numClass);

		for (UINT i = 0;i < numClass;i++) {
			std::string queryCountCounter = "select counter from wordtable where userid='";
			std::string queryFindMean = "select meaning from wordtable where userid='";
			queryCountCounter.append(std::to_string(userid));
			queryCountCounter += "' and classid ='";
			queryFindMean.append(std::to_string(userid));
			queryFindMean += "' and classid ='";
			queryCountCounter.append(std::to_string((int)(1+i)));
			queryCountCounter += "'";
			queryFindMean.append(std::to_string((int)(1 + i)));
			queryFindMean += "'";
			tmpCSQL.clearResult();
			tmpCSQL.queryMYSQL(queryCountCounter.c_str());
			classTracker[i].classLabel = 1 + i;
			classTracker[i].counter = tmpCSQL.resultFirstLineInINT();
			totalNumTrainingExamples += classTracker[i].counter;

			tmpCSQL.clearResult();
			tmpCSQL.queryMYSQL(queryFindMean.c_str());
			classTracker[i].className = tmpCSQL.resultStringLineByLine();
		}

	// file out to _userid_Training_angle/hand_Data.grt
		std::ofstream outFile;
		std::string outFileName = std::to_string(userid);
		if (kkk == 0) { 
			outFileName += "TrainingAngleData.grt";
			numDimen = 6;
		}
		else { 
			outFileName += "TrainingHandData.grt"; 
			numDimen = 10;
		}

		outFile.open(outFileName, std::ios::out | std::ios::trunc);
		outFile << "GRT_LABELLED_TIME_SERIES_CLASSIFICATION_DATA_FILE_V1.0\n";
		outFile << "DatasetName: HandGestureTimeSeriesData\n";
		outFile << "InfoText: Do You Know Kimchi?? \n";
		outFile << "NumDimensions: " << numDimen << "\n";
		outFile << "TotalNumTrainingExamples: " << totalNumTrainingExamples << "\n";
		outFile << "NumberOfClasses: " << numClass << "\n";
		outFile << "ClassIDsAndCounters: \n";
		for (UINT i = 0;i < classTracker.size();i++) {
			outFile << classTracker[i].classLabel << " "
				<< classTracker[i].counter << " "
				<< classTracker[i].className << "\n";
		}
		outFile << "UseExternalRanges: 0\n";
		outFile << "LabelledTimeSeriesTrainingData: \n";

		/*
			여기서 디비 초기화 후 테이블에서 select * from 하기
		*/
		
		for (UINT x = 0;x < numClass;x++) {
			int tmpTSLine = 0;
			for (int y = 1;y <= classTracker[x].counter;y++) {
				tmpTSLine++;
				outFile << "************TIME_SERIES************\n";
				outFile << "ClassID: " << std::to_string(classTracker[x].classLabel) << "\n";
				
				std::string queryGetTSLen = "select tslen from ";
				if (kkk == 0) queryGetTSLen += "angle";
				else queryGetTSLen += "finger";
				queryGetTSLen += "table where uid = '";
				queryGetTSLen.append(std::to_string(userid));
				queryGetTSLen += "' and classid ='";
				queryGetTSLen.append(std::to_string(classTracker[x].classLabel));
				queryGetTSLen += "'and tsid='";
				queryGetTSLen.append(std::to_string(tmpTSLine));
				queryGetTSLen += "'";
				tmpCSQL.clearResult();
				tmpCSQL.queryMYSQL(queryGetTSLen.c_str());
				int tmpLine = tmpCSQL.resultFirstLineInINT();
				
				outFile << "TimeSeriesLength: " << tmpLine << "\n";
				outFile << "TimeSeriesData: \n";
				for (UINT z = 0;z < tmpLine;z++) {
					std::string queryGetTSData = "select ";
					if (kkk == 0) queryGetTSData += "angle";
					else queryGetTSData += "finger";
					queryGetTSData += "data from ";
					if (kkk == 0) queryGetTSData += "angle";
					else queryGetTSData += "finger";
					queryGetTSData += "table where uid = '";
					queryGetTSData.append(std::to_string(userid));
					queryGetTSData += "' and classid = '";
					queryGetTSData.append(std::to_string(classTracker[x].classLabel));
					queryGetTSData += "' and tsid = '";
					queryGetTSData.append(std::to_string(tmpTSLine));
					queryGetTSData += "' and tsline = '";
					queryGetTSData.append(std::to_string((int)(1+z)));
					queryGetTSData += "'";
					tmpCSQL.clearResult();
					tmpCSQL.queryMYSQL(queryGetTSData.c_str());
					std::string returnedTSData = tmpCSQL.resultStringLineByLine();

					outFile << returnedTSData << std::endl;
				}
			}
			std::cout << "inseting classid #" << 1 + x << "done !\n";
		}//for titalnum

		outFile.close();
	}//twice for 0:angle, 1:hand
	std::cout << "wow done!\n";
}

/*
	receive process all in here
*/
int Server::receive(int userid, void *data) {

	ConnectSQL tmpCSQL = ((netData*)data)->sql;
	SOCKET client = ((netData*)data)->client;

	int ClassID = -1;
	int tsid = 0;
	int prevID = 0;
	int TimeSeriesLength = 0;
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	std::ifstream in;
	std::string fname1 = std::to_string(userid);
	std::string fname2 = std::to_string(userid);
	fname1 += "angle.grt";
	fname2 += "hand.grt";

	std::cout << "receiving files start\n";
	// receive *.grt from client
	fileReceive(client, (char*)fname1.c_str());

	fileReceive(client, (char*)fname2.c_str());
	std::cout << "received files\n";

	std::cout << "insert files to db\n";
	// insert *.grt into db;
	in.open(fname1);

	bool newClassID = false;
	//TrainingDataParser angelParse;
	//angelParse.ReadHeader(in);
	
	/*
		여기서 wordtable을 추가
	*/
	int numberOfClasses = 0;
	// word table -> classid*, meaning*, counter*, userid
	std::cout << "debug1\n";

	//bool lengtherror = false;

	while (true) {
		memset(buffer, 0, sizeof(buffer));
		in.getline(buffer, 1024);
		std::stringstream ss(buffer);
		std::string token;
		char tmpBufIn[50];
		getline(ss, token, ':');
		if (strcmp(token.c_str(), "DatasetName") == 0) {
			getline(ss, token, ':');
		}
		else if (strcmp(token.c_str(), "InfoText") == 0) {
			getline(ss, token, ':');
		}
		else if (strcmp(token.c_str(), "NumDimensions") == 0) {
			getline(ss, token, ':');
			//cout << "NumDimensions : " << NumDimensions << endl;
		}
		else if (strcmp(token.c_str(), "TotalNumTrainingExamples") == 0) {
			getline(ss, token, ':');
			//cout << "TotalNumTrainingExamples : " << TotalNumTrainingExamples << endl;
		}
		else if (strcmp(token.c_str(), "NumberOfClasses") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			numberOfClasses = atoi(token.c_str());
			//cout << "NumberOfClasses : " << NumberOfClasses << endl;
		}
		else if (strcmp(token.c_str(), "ClassIDsAndCounters") == 0) {
			int tmp = 0;
			int tmpInt[2];
			//cout << "ClassIDsAndCounters [";
			for (int i = 0;i < numberOfClasses;i++) {
				//	if (i != 0) cout << ", ";
				memset(buffer, 0, sizeof(buffer));
				in >> buffer;	//	remove class id
				tmpInt[0] = atoi(buffer);
				memset(buffer, 0, sizeof(buffer));
				in >> buffer;
				tmpInt[1] = atoi(buffer);
				if (tmpInt[1] == 0) { 
					std::cout << "Length Error\n";
					return -1;
				}
				memset(buffer, 0, sizeof(buffer));
				in >> buffer;
				//	cout << ClassIDsAndCounters[i];
				std::string queryInsertWordTable = "insert into wordtable(classid, meaning, counter, userid) values('";
				queryInsertWordTable.append(std::to_string(tmpInt[0]));
				queryInsertWordTable += "', '";
				queryInsertWordTable.append(buffer);
				queryInsertWordTable += "', '";
				queryInsertWordTable.append(std::to_string(tmpInt[1]));
				queryInsertWordTable += "', '";
				queryInsertWordTable.append(std::to_string(userid));
				queryInsertWordTable += "')";
				tmpCSQL.clearResult();

				if (isThisClassNew(tmpInt[0], userid, data)) {
					tmpCSQL.queryMYSQL(queryInsertWordTable.c_str());
				}
				else {
					std::string queryAddOneCounter = "update wordtable set counter = counter + 1 where classid = '";
					queryAddOneCounter.append(std::to_string(tmpInt[0]));
					queryAddOneCounter += "' and userid='";
					queryAddOneCounter.append(std::to_string(userid));
					queryAddOneCounter += "'";
					tmpCSQL.queryMYSQL(queryAddOneCounter.c_str());
				}

			}
			//cout << "]" << endl;
		}
		else if (strcmp(token.c_str(), "UseExternalRanges") == 0) {
			getline(ss, token, ':');
			//cout << "UseExternalRanges : " << UseExternalRanges << endl;
		}
		else if (strcmp(token.c_str(), "LabelledTimeSeriesTrainingData") == 0) {
			break;
		}
	}
	
	std::cout << "head all read\n";
	// parse & query to db
	// code not in TrainingDataParser but here
	// because we need to query a lot...;


	std::cout << "debug2\n";


	while (in.getline(buffer, 1024)) {
		if (in.eof()) {
			std::cout << "done class #" << prevID << "!\n";
			break;
		}
		std::stringstream ss(buffer);
		std::string token;
		getline(ss, token, ':');
		if (strcmp(token.c_str(), "ClassID") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			if (prevID == 0) {
				ClassID = atoi(token.c_str());

				if (!isThisClassNew(ClassID, userid, data)) {
					std::string queryGetOldCounter = "select counter from wordtable where userid='";
					queryGetOldCounter.append(std::to_string(userid));
					queryGetOldCounter += "' and classid = '";
					queryGetOldCounter.append(std::to_string(ClassID));
					queryGetOldCounter += "'";
					tmpCSQL.clearResult();
					tmpCSQL.queryMYSQL(queryGetOldCounter.c_str());
					tsid = tmpCSQL.resultFirstLineInINT() - 1;
				}

				prevID = ClassID;
			}
			else {
				prevID = ClassID;
				ClassID = atoi(token.c_str());
			}
			if (prevID != ClassID) {
				std::cout << "done class #" << prevID << "!\n";
				tsid = 0;
				std::string query = "insert into wordtable(classid, counter, userid) values(";
				query.append(std::to_string(prevID));
				prevID = 0;
			}
		}
		else if (strcmp(token.c_str(), "TimeSeriesLength") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			TimeSeriesLength = atoi(token.c_str());
		}
		else if (strcmp(token.c_str(), "TimeSeriesData") == 0) {
			tsid++;
			for (int i = 0;i < TimeSeriesLength;i++) {
				memset(buffer, 0, 1024);
				in.getline(buffer, 1024);
				std::string query = "insert into AngleTable(uid, classid, tsid, tslen, tsline, angledata) values('";
				query += std::to_string(userid);
				query += "', '";
				query += std::to_string(ClassID);
				query += "', '";
				query += std::to_string(tsid);
				query += "', '";
				int tmpLine = 1 + i;
				query += std::to_string(TimeSeriesLength);
				query += "', '";
				query += std::to_string(tmpLine);
				query += "', '";
				query += buffer;
				query += "')";
				std::string rQuery = replaceAll(query, "\t", " ");
				tmpCSQL.clearResult();
				tmpCSQL.queryMYSQL(rQuery.c_str());
			}
		}
		memset(buffer, 0, 1024);
	}
	in.close();

	newClassID = false;
	in.open(fname2);
	TrainingDataParser handParse;
	handParse.ReadHeader(in);
	std::cout << "head all read\n";


	std::cout << "debug3\n";
	// parse & query to db
	// code not in TrainingDataParser but here
	// because we need to query a lot...
	prevID = 0;
	tsid = 0;
	while (in.getline(buffer, 1024)){
		
		if (in.eof()) {
			std::cout << "done class #" << prevID << "!\n";
			break;
		}
		std::stringstream ss(buffer);
		std::string token;
		getline(ss, token, ':');
		if (strcmp(token.c_str(), "ClassID") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			if (prevID == 0) {
				ClassID = atoi(token.c_str());
				if (!isThisClassNew(ClassID, userid, data)) {
					std::string queryGetOldCounter = "select counter from wordtable where userid='";
					queryGetOldCounter.append(std::to_string(userid));
					queryGetOldCounter += "' and classid = '";
					queryGetOldCounter.append(std::to_string(ClassID));
					queryGetOldCounter += "'";
					tmpCSQL.clearResult();
					tmpCSQL.queryMYSQL(queryGetOldCounter.c_str());
					tsid = tmpCSQL.resultFirstLineInINT() - 1;
				}
				prevID = ClassID;
			}
			else {
				prevID = ClassID;
				ClassID = atoi(token.c_str());
			}
			if (prevID != ClassID) {
				std::cout << "done class #" << prevID << "!\n";
				tsid = 0;
			}
		}
		else if (strcmp(token.c_str(), "TimeSeriesLength") == 0) {
			getline(ss, token, ':');
			token.erase(token.find_last_not_of(" ") + 1);
			TimeSeriesLength = atoi(token.c_str());
		}
		else if (strcmp(token.c_str(), "TimeSeriesData") == 0) {
			tsid++;
			for (int i = 0;i < TimeSeriesLength;i++) {
				memset(buffer, 0, 1024);
				in.getline(buffer, 1024);
				std::string query = "insert into FingerTable(uid, classid, tsid, tslen, tsline, fingerdata) values('";
				query += std::to_string(userid);
				query += "', '";
				query += std::to_string(ClassID);
				query += "', '";
				query += std::to_string(tsid);
				query += "', '";
				int tmpLine = 1 + i;
				query += std::to_string(TimeSeriesLength);
				query += "', '";
				query += std::to_string(tmpLine);
				query += "', '";
				query += buffer;
				query += "')";
				std::string rQuery = replaceAll(query, "\t", " ");
				tmpCSQL.clearResult();
				tmpCSQL.queryMYSQL(rQuery.c_str());
			}
		}
		memset(buffer, 0, 1024);
	} 
	in.close();
	std::cout << "insert files to db done\n";

	//CreateModelFile(userid);

	return 0;
}

/* @ ====================
 * Server::withClient()
 * ====================*/
/*
 * Function : void withClient(void*)
 *
 * Decription :
 *  this function runs when one client is accepted
 *
 *  login to mysql server  
 *  
 *  run 'login()' when client send id, pw 
 * 
 *  after successful login, now it's ready to receive commands
 *  
 * Availible commands :
 *  FileSend, start sending model.grt to client
 *  FileReceive, start receiving train.grt from client
 *  EndConnection, end connection with client
 *  
 * Returns :
 *  void
 */
void Server::withClient(
	void *data				/* ((SOCKET*)data) */
	) {

	ConnectSQL cSQL(data);
	SOCKET *cl = (SOCKET*)data;
	SOCKET CLI = *cl;
	
	netData nda;
	nda.sql = cSQL;
	nda.client = CLI;

	char rec[50];
	memset(rec, 0, sizeof(rec));

	// mysql connection
	if (cSQL.connectMYSQL())return;

	// using auto mutexing, surround with { }
	{
		std::lock_guard<std::mutex> iolock(iomutex);
		std::cout << "Client connected, thread #" <<
			GetCurrentThreadId() << " is running\n";
	}

	// login client
	int userID = login((void*)&nda);
	if (userID < 0) {
		std::lock_guard<std::mutex> iolock(iomutex);
		std::cout << "Client connected, thread #"
			<< GetCurrentThreadId() << " login fail\n";
		return;
	}
	else {
		std::lock_guard<std::mutex> iolock(iomutex);
		std::cout << "Client connected, thread #"
			<< GetCurrentThreadId() <<" login done\n";
	}

	// receive commands from client
	while (true) {
		memset(rec, 0, sizeof(rec));
		int sz = recv(CLI, rec, 32, 0);
		std::cout << rec << std::endl;

		if (strcmp(rec, "") == 0) break;

		if (strcmp(rec, "FileSend") == 0) {
			// client wants to send file to me

			sendData(CLI, "OK\n");

			//std::thread recvThd = runDBRefresh(userID, &CLI);
			//recvThd.detach();
			receive(userID, &nda);

			//std::thread makingModel = runGRT(userID);
			//makingModel.detach();
		}
		else if (strcmp(rec, "CreateModel") == 0) {
			//GRT::TimeSeriesClassificationData tscDATA;
			//std::string newTrainingFileName = std::to_string(userID);
			//newTrainingFileName += "TrainingAngleData.grt";
			//tscDATA.saveDatasetToFile(newTrainingFileName);

			std::thread grtmaking = runGRT(userID, &nda);
			grtmaking.detach();
		}
		else if (strcmp(rec, "DeleteClass") == 0) {
			;
		}
		else if (strcmp(rec, "FileReceive") == 0) {
			// client wants to receive file from me

			std::string fname;
			fname = std::to_string(userID);
			fname += "ModelAngleData.grt";
			fileSend(CLI, (char*)fname.c_str());

			fname.clear();
			fname = std::to_string(userID);
			fname += "ModelHandData.grt";
			fileSend(CLI, (char*)fname.c_str());
		}
		else if (strcmp(rec, "EndConnection") == 0) {
			// client done all work, wants to disconnect

			{
				std::lock_guard<std::mutex> iolock(iomutex);
				std::cout << "Client thread #"
					<< GetCurrentThreadId() 
					<< " connection end\n";
			}
			break;
		}
	}
	
	
	//receive(userID, &nda);
	//readTrainFromDB(userID, &nda);
	//std::thread grtthread= runGRT(userID);
	//grtthread.detach();

	// disconnect	
	// sendData(CLI, "OK!!!!");
	cSQL.closeMYSQL();
	closesocket(CLI);
	{
		std::lock_guard<std::mutex> iolock(iomutex);
		std::cout << "Client thread #" <<
			GetCurrentThreadId() << " disconnected\n";
	}
	return;
}

/* @ ====================
* Server::login()
* ====================*/
/*
* Function : int login(void*)
*
* Decription :
*  login to mysql server
*
*  receive id and pw from client
*  if valid, login.
*
*  if the password is wrong, function fails
*
*  if there is no id, create user infomation in database
*
*  after successful login, now it's ready to receive commands
*
* Returns :
*  userID from database when it's successful
*  if not, return -1
*/
int Server::login(
	void* data				/* ((netData*)data) */
	) {
	ConnectSQL tmpCSQL = ((netData*)data)->sql;
	SOCKET tmpClient = ((netData*)data)->client;

	char idLenBuf[4], pwLenBuf[4],
		idBuf[20], pwBuf[20];
	int idLen, pwLen;

	memset(idLenBuf, 0, sizeof(idLenBuf));
	memset(pwLenBuf, 0, sizeof(pwLenBuf));
	memset(idBuf, 0, sizeof(idBuf));
	memset(pwBuf, 0, sizeof(pwBuf));

	int recs = recvData(tmpClient, idLenBuf, 3);
	idLen = atoi(idLenBuf);
	recvData(tmpClient, idBuf, idLen);

	recs = recvData(tmpClient, pwLenBuf, 3);
	pwLen = atoi(pwLenBuf);
	recvData(tmpClient, pwBuf, pwLen);

	int userid = tmpCSQL.LoginJoin(idBuf, pwBuf);
	if (userid < 0)
		sendData(tmpClient, "NO\n");
	else
		sendData(tmpClient, "OK\n");
	return userid;
}

/* @ ====================
* Server::CreateModelFile()
* ====================*/
/*
* Function : void CreateModelFile(int*)
*
* Decription :
*  create model.grt
*
* Returns :
*  void
*/
void Server::CreateModelFile(
	int id,					/* userID */
	void *data
	) {

	{
		std::lock_guard<std::mutex> iolock(iomutex);
		std::cout << "Thread #"
			<< GetCurrentThreadId() << " creating model file \n";
	}

	ConnectSQL tmpCSQL = ((netData*)data)->sql;
	SOCKET client = ((netData*)data)->client;
	
	readTrainFromDB(id, data);

	GRT::Dyslexic grt(std::to_string(id));
	grt.loadTrainingData();
	grt.train();

	sendData(client, "OK\n");
	{
		std::lock_guard<std::mutex> iolock(iomutex);
		std::cout << "Thread #"
			<< GetCurrentThreadId() << " created model file \n";
	}
}

/* @ ====================
* Server::runGRT()
* ====================*/
/*
* Function : thread runGRT(int)
*
* Decription :
*  create a thread running 'CreateModelFile()'
*
* Returns :
*  created thread
*/
std::thread Server::runGRT(
	int id,					/* userID */
	void *data
	) {
	return std::thread([=] {CreateModelFile(id, data);});
}

/* @ ====================
* Server::runThread()
* ====================*/
/*
* Function : thread runThread()
*
* Decription :
*  create a thread running 'withClient()'
*
* Returns :
*  created thread
*/
std::thread Server::runThread() {
	SOCKET CLI = returnClientSock();
	return std::thread([=] {withClient((void*)&CLI);});
}

std::thread Server::runDBRefresh(int classid, void *cli) {
	return std::thread([=] {receive(classid, cli);});
}

/* @ ====================
* Server::returnClientSock()
* ====================*/
/*
* Function : SOCKET returnClientSock()
*
* Decription :
*  used in 'runThread()', which needs client socket data
*
* Returns :
*  client socket
*/
SOCKET Server::returnClientSock() {
	return clientSock;
}

bool Server::isThisClassNew(int classid, int userid, void *data) {
	ConnectSQL tmpCSQL = ((netData*)data)->sql;
	SOCKET client = ((netData*)data)->client;

	std::string query = "select count(*) from wordtable where userid = '";
	query.append(std::to_string(userid));
	query += "' and classid = '";
	query.append(std::to_string(classid));
	query += "'";

	tmpCSQL.clearResult();
	tmpCSQL.queryMYSQL(query.c_str());
	int count = tmpCSQL.resultFirstLineInINT();

	if (count) return false;	// count == 1 means there is one. so, not new
	else return true;			// count == 0 means there isn't one. so, new!
}


/* @ ====================
* Server::printRibbon()
* ====================*/
/*
* Function : void printRibbon()
*
* Decription :
*  print a pretty ribbon
*
* Returns :
*  void
*/
void Server::printRibbon() {
	std::cout <<
		"  iitE                 Ejii  \n" <<
		"  iitfDGW           WGEftii  \n" <<
		"  jttfGGLfD       EfLGGfttj  \n" <<
		"   ttLGGLftj     jtfLKGftt   \n" <<
		"   ttLGGLfjtDGGGGitfLGDLtt   \n" <<
		"  jttLGGLjtiEEEEE;tjLGGLttf  \n" <<
		"  tttGGGfjt;GGKGG;tjfGGGtti  \n" <<
		"  ttjGGGfjt,jffff;ijfWGGjtt  \n" <<
		" LttfGGLfti,ttttt;itfLGDftt  \n" <<
		" itLK     tiiiiiiit,   ;Wjti \n" <<
		" t.........i:   :i.........j \n" <<
		" :::::::t,,,i   i,,;i::::::: \n" <<
		"    LGj;;;;;     ;;;;;jfjL   \n" <<
		"      tiiiii     iiiiit      \n" <<
		"     Wtttttf     fttttt      \n" <<
		"     jjjjtt       ttjjjL     \n" <<
		"    fffffjj       jjfffff    \n" <<
		"  WfffffffK       KfffffffK  \n" <<
		" Wfjtttfff         fffjttjfW \n" <<
		"       tfL         Lft       \n" <<
		"       tfW         Wft       \n" <<
		"        W           W        \n" << std::endl;

	return;
}

/* @ ====================
* Server::printSSM()
* ====================*/
/*
* Function : void printSSM()
*
* Decription :
*  print fantastic SSM logo
*
* Returns :
*  void
*/
void Server::printSSM() {
	std::cout <<
		"          DLj           S      \n" <<
		"         D G:G          a      \n" <<
		"       .D  G  D         m      \n" <<
		"      i.   L   fi       s      \n" <<
		"     L;    G    ;f      u      \n" <<
		"    D      L      j     n      \n" <<
		"    f,    G.f    ,i     g S    \n" <<
		"     tj  G  .D  ji        o    \n" <<
		"      :tG     GG.         f    \n" <<
		"      t.G    :Gtt         t    \n" <<
		"     G:  D. if  :L        w    \n" <<
		"    G     Gj,     G       a    \n" <<
		"   f;   .D.L    ;f        r    \n" <<
		"     ;L  L   D  f;      M e    \n" <<
		"       G,     ;L        e      \n" <<
		"      f L    :Lif       m      \n" <<
		"     G.  G: ji  .Gi     b      \n" <<
		"    t     .D      G     e      \n" <<
		"    ft     L     t:     r      \n" <<
		"     ,L    L    L,      s      \n" <<
		"       D.  G  .f        h      \n" <<
		"        L  G if         i      \n" <<
		"         L:Lf;          p      \n";
	return;
}


std::string replaceAll(const std::string &str, const std::string &pattern, const std::string &replace) {
	std::string result = str;
	std::string::size_type pos = 0;
	std::string::size_type offset = 0;

	while ((pos = result.find(pattern, offset)) != std::string::npos) {
		result.replace(result.begin() + pos, result.begin() + pos + pattern.size(), replace);
		offset = pos + replace.size();
	}

	return result;
}