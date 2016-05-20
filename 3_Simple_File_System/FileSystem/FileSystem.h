#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_



//CMD
#define CMD_NUM 15
#define CMD_LEN 10
static const char[CMD_NUM][CMD_LEN] = {
	"mkdir", "rmdir", "ls", "cd", "create",
  	"rm", "open", "close", "write", 
  	"read", "exit", "help","bigfile"
}

// Write Style
#define WS_REWRITE 0
#define WS_OVERWRITE 1
#define WS_APPEND 2 

#endif
