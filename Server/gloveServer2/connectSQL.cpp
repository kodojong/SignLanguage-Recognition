#include "connectSQL.h"
#include "global.h"

#include <iostream>


ConnectSQL::ConnectSQL() {
}

ConnectSQL::ConnectSQL(void *data) {
	client = (SOCKET *)data;
	CLIENT = *client;
	mysql = mysql_init(NULL);
	mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
	mysql_options(mysql, MYSQL_INIT_COMMAND, "set names utf8");
}



ConnectSQL::~ConnectSQL() {
	mysql_free_result(result);
	//mysql_close(mysql);

	//closesocket(CLIENT);
	//delete client;
}

int ConnectSQL::connectMYSQL() {
	if (!mysql_real_connect(mysql, DB_HOST, DB_USER, DB_PASS, DB_NAME,
		3306, (char *)NULL, 0)) {
		std::cerr << mysql_error(mysql);
		closesocket(CLIENT);
		return 1;
	}
	return 0;
}

int ConnectSQL::LoginJoin(char* id, char* pw) {
	std::string query = "select count(*) from usertable where loginID = '";
	MYSQL_ROW tmpRow;
	query.append(id);
	query += "'";
	queryMYSQL(query.c_str());
	tmpRow = mysql_fetch_row(result);
	if (strcmp(tmpRow[0], "0") == 0) {
		// there is no such id. join
		mysql_free_result(result);
		query.clear();
		query = "insert into usertable(loginid, loginpw), values('";
		query.append(id);
		query += "', sha1(";
		query.append(pw);
		query += "))";
		queryMYSQL(query.c_str());
		tmpRow = mysql_fetch_row(result);
		int returnIndex = atoi(tmpRow[0]);
		return returnIndex;
	}
	else {
		// there is an id. check pw
		mysql_free_result(result);
		query += "and loginpw = sha1(";
		query.append(pw);
		query += ")";
		queryMYSQL(query.c_str());
		tmpRow = mysql_fetch_row(result);
		if (strcmp(tmpRow[0], "0") == 0) {
			// password no match
			return -1;
		}
		else {
			// id, pw match. login
			mysql_free_result(result);
			query.clear();
			query = "select id from usertable where loginid ='";
			query.append(id);
			query += "' and loginpw = sha1(";
			query.append(pw);
			query += ")";
			queryMYSQL(query.c_str());
			tmpRow = mysql_fetch_row(result);
			int returnIndex = atoi(tmpRow[0]);
			return returnIndex;
		}
		//check for password
	}
	
}


std::string ConnectSQL::resultMYSQL() {
	resultString = "";
	std::string query = "select * from usertable where id = 3";
	if (queryMYSQL(query.c_str())) {
		return nullptr;
	}

	row = mysql_fetch_row(result);
	resultString += row[0];

	for (int i = 1;i < 5;i++) {
		if (row[i] = 0) break;
		resultString += " ";
		resultString += row[i];
	}
	resultString += "\0";
	return resultString;
}

int ConnectSQL::queryMYSQL(const char *query) {
	//std::cout << "Query : " << query << std::endl;
	if (mysql_query(mysql, query)) {
		std::cout << "Query : " << query << std::endl;
		std::cerr << mysql_error(mysql) << std::endl;
		mysql_close(mysql);
		return 1;
	}
	result = mysql_use_result(mysql);
	return 0;
}

void ConnectSQL::sendBack() {

	send(CLIENT, "hello Godbriel!", 15, 0);
	//send(CLIENT, resultString.c_str(), resultString.size() + 1, 0);
}

void ConnectSQL::closeMYSQL() {
	mysql_close(mysql);
}

int ConnectSQL::clearResult() {
	mysql_free_result(result);
	return 0;
}

int ConnectSQL::resultFirstLineInINT() {
	int value = 0;

	row = mysql_fetch_row(result);
	resultString.clear();
	resultString += row[0];

	value = atoi(resultString.c_str());

	return value;
}

int ConnectSQL::resultNextLine() {
	int done = 0;

	return done;
}

std::string ConnectSQL::resultStringLineByLine() {
	row = mysql_fetch_row(result);
	std::string returnString;
	returnString += row[0];

	return returnString;
}