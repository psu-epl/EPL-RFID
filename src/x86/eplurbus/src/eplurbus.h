#ifndef _EPLURBUS_
#define _EPLURBUS_

#include <string>
#include <iostream>
#include <fstream>
#include <queue>



using namespace std;

const int bitwidth = 64;
const int streamSize = 10;

typedef enum 
{ 
	status_success = 0, 
	status_failure 
} exit_status;

class EPLurbus
{
	public:
		EPLurbus();
		~EPLurbus();

		exit_status openFile(string filename);
		exit_status fillBuffers();
		exit_status displayBuffers();
		exit_status closeFile();
	
	private:
		//queue <int> fd;
		const int m_size;
		uint64_t *m_pRawBitstreamBuffer;
		string *m_pStringBitstreamBuffer[bitwidth];
		
		ifstream fin; 
		
		int m_size26;
		int m_size34;
		int m_size35;
		int m_size37;
		int m_size40;

		uint64_t *m_pBuff26;
		uint64_t *m_pBuff34;
		uint64_t *m_pBuff35;
		uint64_t *m_pBuff37;
		uint64_t *m_pBuff40;

		// 26, 34, 35, 37, 40

};

#endif // _EPLURBUS_ 

