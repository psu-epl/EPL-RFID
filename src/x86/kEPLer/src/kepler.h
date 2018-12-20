#ifndef _KEPLER_
#define _KEPLER_

#include <string>
#include <iostream>
#include <fstream>
#include <bitset>
#include <algorithm>
#include <list>

using namespace std;

const int kBitWidth = 64;
const int kStreamLength = 8;
const int kBits26 = 26;
const int kBits34 = 34;
const int kBits35 = 35;
const int kBits37 = 37;
const int kBits40 = 40;
const int kMaxOpenFiles = 5;

typedef enum 
{ 
	status_success = 0, 
	status_failure 
} exit_status;

template<size_t number_of_bits>
class Bitz
{
  public:
    Bitz(int totalBits);
    Bitz(uint64_t *pBuff,int mStreamBufferLength,int bitWidth);
    ~Bitz();
    
    exit_status convertBuffer(uint64_t *pStreamBuffer,int streamBufferLength);
    void display();
    int getBitBufferLength() const;

  private:
    int mBitBufferLength;
    bitset<number_of_bits> *mpBitBuffer;
    int shift; 
    int slack;
};

class Kepler
{
	public:
		Kepler();
		~Kepler();

		exit_status openFile(string filename);
		exit_status openFile2(string filename);
		exit_status openFile3(string filename);
		exit_status fillBuffers();
    exit_status shiftLeft();
   
    template<size_t number_of_bits>
    exit_status convertBuffer(
      bitset<number_of_bits> *pBuff, 
      int buffLength
    );
	
    exit_status displayBuffers();
		exit_status closeFile();
    exit_status closeFile2();
    exit_status closeFile3();
	
	private:
    const int mStreamLength;
		const int mStreamBufferLength;
    uint64_t *mpStreamBuffer;
		int mOpenFileCount;
		
    ifstream fin;
    ifstream mFin[kMaxOpenFiles];
    list<ifstream *> mFin2;
		
		// 26, 34, 35, 37, 40
    Bitz<kBits26> *mpBitz26;
    Bitz<kBits34> *mpBitz34;
    Bitz<kBits35> *mpBitz35;
    Bitz<kBits37> *mpBitz37;
    Bitz<kBits40> *mpBitz40;
};

#endif // _EPLURBUS_ 

