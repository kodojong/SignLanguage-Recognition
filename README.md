# SignLanguage
Sign Language Server &amp; Client

# Server
VC++ directory

> "Include Directories" add ?..\MySQL\MySQL Server 5.7\include

> "Library Directories" add ?..\MySQL\MySQL Server 5.7\lib

Linker

> "Input" add libmysql.lib <- add file in project directory

code...

> add "#pragma comment(lib, "libmysql.lib")" for database

> add "#pragma comment(lib, "Ws2_32.lib")" for winsock
