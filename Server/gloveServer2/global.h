#pragma once

#pragma comment(lib, "libmysql.lib")	// database
#pragma comment(lib, "Ws2_32.lib")		// winsock
#pragma warning(disable: 4819)

#define GLOVE_PORT "4389"				// swssm default port
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "dota2"
#define DB_NAME "gloves"

#define WHATU_DEBUG 0
#define WHATU_COUT	1