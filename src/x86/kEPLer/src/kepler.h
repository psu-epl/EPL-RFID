#ifndef _KEPLER_
#define _KEPLER_

#include <string>
#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

const int bitwidth = 64;
const int streamLength = 8;
const int bits26 = 26;
const int bits34 = 34;
//const int bits35 = 35;
//const int bits37 = 37;
//const int bits40 = 40;

typedef enum 
{ 
	status_success = 0, 
	status_failure 
} exit_status;

class Kepler
{
	public:
		Kepler();
		~Kepler();

		exit_status openFile(string filename);
		exit_status fillBuffers();
    exit_status shiftLeft();
   
    void shiftItAll(uint64_t *pBuff, size_t n, int shift);
    
    template<size_t number_of_bits>
    exit_status convertBuffer(
      bitset<number_of_bits> *pBuff, 
      int buffLength
    );
		
    exit_status displayBuffers();
		exit_status closeFile();
	
//    void printShift();
//    void printStreamBuffer(uint64_t *pBuff);
	private:
    const int mStreamLength;
		const int mStreamBufferLength;
    uint64_t *mpStreamBuffer;
		
		ifstream fin; 
		
		int mBuff26Length;
		bitset<bits26> *mpBuff26;
		
    int mBuff34Length;
		bitset<bits34> *mpBuff34;
//		int m_size35;
//		int m_size37;
//		int m_size40;

		//uint64_t *m_pBuff26;
//		bitset<bits34> *m_pBuff34;
//		bitset<bits35> *m_pBuff35;
//		bitset<bits37> *m_pBuff37;
//		bitset<bits40> *m_pBuff40;

		// 26, 34, 35, 37, 40

};

#endif // _EPLURBUS_ 

