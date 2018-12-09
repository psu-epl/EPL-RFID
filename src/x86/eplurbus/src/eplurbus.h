#ifndef _EPLURBUS_
#define _EPLURBUS_

#include <string>
#include <iostream>
#include <queue>

enum 
{ 
	status_success = 0, 
	status_failure 
};


{
		
};

class EPLurbus
{
	public:
		EPLurbus();
		~EPLurbus();
		int openFile(string filename);
		int readFile(int size);

	private:
		//queue <int> fd;
		int fd;
		static int size;
		uint64_t rawBuffer[size];
		
		int size26;
		int size34;
		int size35;
		int size37;
		int size40;

		uint64_t *pBuff26;
		uint64_t *pBuff34;
		uint64_t *pBuff35;
		uint64_t *pBuff37;
		uint64_t *pBuff40;

		// 26, 34, 35, 37, 40

};

#endif // _EPLURBUS_ 

