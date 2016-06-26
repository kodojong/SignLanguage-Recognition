#pragma once

#include <mysql.h>
#include <string>

class ConnectSQL {
	SOCKET* client;
	SOCKET CLIENT;

	MYSQL *mysql;
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::string resultString;

public:
	ConnectSQL();
	ConnectSQL(void *data);
	~ConnectSQL();

	int connectMYSQL();
	int LoginJoin(char* id, char* pw);
	std::string resultMYSQL();
	int queryMYSQL(const char* query);
	void sendBack();
	void closeMYSQL();
	
	int clearResult();

	int resultFirstLineInINT();

	int resultNextLine();
	std::string resultStringLineByLine();
};

