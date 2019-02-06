#ifndef _KEPLER_
#define _KEPLER_

#include <string>
#include <iostream>
#include <fstream>
#include <bitset>
#include <algorithm>
#include <list>
#include <iomanip>

using namespace std;

const int kBitWidth = 64;
const int kStreamLength = 8;
const int kMaxOpenFiles = 5;

const int kBits26 = 26;
const int kBits34 = 34;
const int kBits35 = 35;
const int kBits37 = 37;
const int kBits40 = 40;
const int kBits64 = 64;

const int kShiftRange = 64;

typedef enum 
{ 
	status_success = 0, 
	status_failure 
} exit_status;

template<size_t number_of_bits>
class Bitz
{
  public:
    Bitz(uint64_t *pBuff,int mStreamBufferLength,int bitWidth);
    ~Bitz();
    
    exit_status convertBuffer(uint64_t *pStreamBuffer,int streamBufferLength);
    void display();
    void cleanDisplay();
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
    
    template<int number_of_bits>
    exit_status analyzeBuffer();
    
    exit_status analyzeBuffers();
    exit_status shiftLeft();
 
    exit_status shiftBy(uint64_t *pDest,uint64_t *pSource,int buffLength,int shift);
	
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
    Bitz<kBits64> *mpBitz64;
};

//*
template<int number_of_bits>
exit_status Kepler::analyzeBuffer()
{
  Bitz<number_of_bits> *pBitz = NULL;

  uint64_t *pShiftedStreamBuffer = NULL;
 
  cout << "\n\nBitwidth: " << number_of_bits;
  cout << "\nNumber of bits left shifted\n";
  cout << "|\n";
  cout << "v";
  for(int i = 0;i < kShiftRange;++i)
  {
    cout << "\n" << setw(2) << i << ": ";
    //cout << "\n\nShift: " << i;
    pShiftedStreamBuffer = new uint64_t[mStreamBufferLength];
    shiftBy(pShiftedStreamBuffer,mpStreamBuffer,mStreamBufferLength,i);
    
    try
    {
      pBitz = new Bitz<number_of_bits>(
        pShiftedStreamBuffer, 
        mStreamBufferLength, 
        kBitWidth
      );
    }
    catch (std::bad_alloc& ba)
    {
      std::cerr << "bad_alloc caught in analyzeBuffers: " << ba.what() << '\n';
      abort();
    }

    pBitz->display();
    
    delete [] pShiftedStreamBuffer;
    delete pBitz;
  }
	return status_success;
}
//*/

#endif // _EPLURBUS_ 

