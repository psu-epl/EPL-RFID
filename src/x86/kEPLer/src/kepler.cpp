#include "kepler.h"

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>

using namespace std;

void shiftItAll(uint64_t *pBuff,int buffLength, int nBits, int shift)
{
  for(int j = 0; j < buffLength;++j)
  {
    pBuff[j] = (pBuff[j] << nBits) | (pBuff[j+1] >> shift);
  }
}

template<size_t number_of_bits>
Bitz<number_of_bits>::Bitz(
  uint64_t *pStreamBuffer, 
  int streamBufferLength, 
  int bitWidth
) : mpBitBuffer(NULL)
{
  //*
  shift = (bitWidth - number_of_bits);
  
  int totalBits = streamBufferLength * bitWidth; 
  mBitBufferLength = totalBits / number_of_bits;
  slack = totalBits - (mBitBufferLength * number_of_bits);

  try
  {
    if(!mpBitBuffer)
    {
      mpBitBuffer = new bitset<number_of_bits>[mBitBufferLength];
    }
    else
    {
      abort();
    }
    uint64_t *pTempStreamBuffer = new uint64_t[streamBufferLength];
    memcpy(pTempStreamBuffer, pStreamBuffer, sizeof(uint64_t) * streamBufferLength);
    convertBuffer(pTempStreamBuffer,streamBufferLength);
    delete [] pTempStreamBuffer;
  }
  catch (std::bad_alloc& ba)
  {
    std::cerr << "bad_alloc caught: " << ba.what() << '\n';
    abort(); 
  }
  //*/
}

template<size_t number_of_bits>
Bitz<number_of_bits>::~Bitz()
{ 
  if(mpBitBuffer)
  {
    delete mpBitBuffer;
  }
}

template<size_t number_of_bits>
exit_status Bitz<number_of_bits>::convertBuffer(
  uint64_t *pStreamBuffer,
  int streamBufferLength
)
{
  if(!pStreamBuffer)
  {
    return status_failure;
  }

  for(int i = 0;i < mBitBufferLength;++i)
  { 
    if(number_of_bits < kBitWidth)
    {
      mpBitBuffer[i] = pStreamBuffer[0] >> shift;
      shiftItAll(pStreamBuffer, streamBufferLength, number_of_bits, shift);
    }
    else
    {
      mpBitBuffer[i] = pStreamBuffer[i];
    }
  }

  return status_success;
}

template<size_t number_of_bits>
void Bitz<number_of_bits>::cleanDisplay()
{
  for(int i = 0;i < mBitBufferLength;++i)
  {
    cout << bitset<number_of_bits>(mpBitBuffer[i]); 
  }

	for(int i = 0;i < slack; ++i)
	{
    cout << "*";	
	}
}

template<size_t number_of_bits>
void Bitz<number_of_bits>::display()
{
//*
  cout << '\n';
	
  for(int i = 0;i < mBitBufferLength;++i)
  {
    cout << "|";
    for(size_t j = 0;j < number_of_bits - 1;++j)
    {
      cout << ' ';
    }
  }

  cout << '\n';
  
  cleanDisplay();
//*/
}

template<size_t number_of_bits>
int Bitz<number_of_bits>::getBitBufferLength() const
{
  return mBitBufferLength;
}

Kepler::Kepler() : 
  mStreamLength(kStreamLength),
  mStreamBufferLength(kStreamLength/2),
  mOpenFileCount(0)
{
	mpStreamBuffer = new uint64_t[mStreamBufferLength];
 
  mpBitz26 = NULL;
  mpBitz34 = NULL;
  mpBitz35 = NULL;
  mpBitz37 = NULL;
  mpBitz40 = NULL;
  mpBitz64 = NULL;
}

Kepler::~Kepler()
{
  if(mpStreamBuffer)
  {
	  delete mpStreamBuffer;
  }
  if(mpBitz26)
  {
    delete mpBitz26;
  }
  if(mpBitz34)
  {
    delete mpBitz34;
  }
  if(mpBitz35)
  {
    delete mpBitz35;
  }
  if(mpBitz37)
  {
    delete mpBitz37;
  }
  if(mpBitz40)
  {
    delete mpBitz40;
  }
  if(mpBitz64)
  {
    delete mpBitz64;
  }
}

exit_status Kepler::openFile(string filename)
{
	fin.open(filename);
	if(fin.fail())
	{
		cout << "Open file failure: " << filename << "\n";
		return status_failure;
	}

	return status_success;
}

//*
exit_status Kepler::openFile2(string filename)
{
  if(mOpenFileCount >= kMaxOpenFiles)
  {
    cout << "Maximum number of files are open...\n";
    return status_failure;
  }
  int tempFileCount = mOpenFileCount + 1; 
	mFin[tempFileCount].open(filename);

	if(mFin[tempFileCount].fail())
	{
		cout << "Open file failure: " << filename << "\n";
		return status_failure;
  }

  ++mOpenFileCount;

	return status_success;
}
//*/

//*
exit_status Kepler::openFile3(string filename)
{
  ifstream *pFin = new ifstream;
  pFin->open(filename);

  
	if(pFin->fail())
	{
		cout << "Open file failure: " << filename << "\n";
		return status_failure;
  }

  mFin2.push_front(pFin);
	return status_success;
}
//*/

exit_status Kepler::closeFile()
{
	fin.close();
	return status_success;
}

exit_status Kepler::closeFile2()
{
  for(int i = 0;i < mOpenFileCount;++i)
  {
    if(mFin[i])
    {
      mFin[i].close();
    }
  }
	return status_success;
}

exit_status Kepler::closeFile3()
{
  for(auto const &f : mFin2) {
     f->close(); 
  }
	return status_success;
}

/*
template<size_t number_of_bits>
exit_status Kepler::convertBuffer(
  bitset<number_of_bits> *pBuff, 
  int buffLength
)
{
  if(!mpStreamBuffer)
  {
    return status_failure;
  }

  uint64_t *pStreamBuffer = new uint64_t[mStreamBufferLength];
  memcpy(pStreamBuffer,mpStreamBuffer,sizeof(uint64_t) * mStreamBufferLength);
  
  int shift = kBitWidth - number_of_bits;
  
  for(int i = 0;i < buffLength;++i)
  { 
    pBuff[i] = pStreamBuffer[0] >> shift;
    shiftItAll(pStreamBuffer, mStreamBufferLength, number_of_bits, shift);
  }

  delete [] pStreamBuffer;
  
  return status_success;
}
//*/

exit_status Kepler::fillBuffers()
{
  uint32_t a = 0x0;
	uint64_t bits = 0x0; 

  for(int i = 0;fin >> hex >> a && i < mStreamLength;++i)
	{
    // cat 32 bit input chunks into 64 bit chuncks
    if(i % 2 == 0)
    {
      bits = ((uint64_t) a) << 32;  
    }
    else
    {
      bits |= a;
		  mpStreamBuffer[i/2] = bits;
      bits = 0x0;
    }
	}

  mpBitz26 = new Bitz<kBits26>(mpStreamBuffer, mStreamBufferLength, kBitWidth);
  mpBitz34 = new Bitz<kBits34>(mpStreamBuffer, mStreamBufferLength, kBitWidth);
  mpBitz35 = new Bitz<kBits35>(mpStreamBuffer, mStreamBufferLength, kBitWidth);
  mpBitz37 = new Bitz<kBits37>(mpStreamBuffer, mStreamBufferLength, kBitWidth);
  mpBitz40 = new Bitz<kBits40>(mpStreamBuffer, mStreamBufferLength, kBitWidth);
  mpBitz64 = new Bitz<kBits64>(mpStreamBuffer, mStreamBufferLength, kBitWidth);
	
  return status_success;
}

exit_status Kepler::shiftBy(
  uint64_t *pDest, 
  uint64_t *pSource, 
  int buffLength, 
  int shift
)
{
  if(!pSource)
  {
    return status_failure;
  }

  uint64_t *pBuff = new uint64_t[buffLength];
  memcpy(pBuff,pSource,sizeof(uint64_t) * buffLength);
  
  for(int i = 0;i < buffLength;++i)
  { 
    pDest[i] = pBuff[i] << shift | pBuff[i + 1] >> 63;
  }

  delete [] pBuff;

  return status_success;
}

exit_status Kepler::analyzeBuffers()
{
  Bitz<kBits64> *pBitz64 = NULL;
  Bitz<kBits26> *pBitz26 = NULL;
  Bitz<kBits34> *pBitz34 = NULL;
  Bitz<kBits35> *pBitz35 = NULL;
  Bitz<kBits37> *pBitz37 = NULL;
  Bitz<kBits40> *pBitz40 = NULL;
 
  uint64_t *pShiftedStreamBuffer = NULL;
  
  int kNumberOfBits = 5;
  for(int i = 0;i < kNumberOfBits;++i)
  {
    cout << "\nShifting " << i << " bits\n";
    pShiftedStreamBuffer = new uint64_t[mStreamBufferLength];
    shiftBy(pShiftedStreamBuffer,mpStreamBuffer,mStreamBufferLength,i);

    for(int i = 0;i < mStreamBufferLength;++i)
    {
      cout << "|";
      
      for(int j = 0;j < kBitWidth - 1;++j)
      {
        cout << ((j == (kBitWidth/2 - 1)) ? "." : " ");
      }
    }
    
    cout << '\n';
    
    for(int i = 0;i < mStreamBufferLength;++i)
    {
      cout << bitset<kBitWidth>(mpStreamBuffer[i]); 
    }
    try
    {
      pBitz64 = new Bitz<kBits64>(pShiftedStreamBuffer, mStreamBufferLength, kBitWidth);
      pBitz26 = new Bitz<kBits26>(pShiftedStreamBuffer, mStreamBufferLength, kBitWidth);
      pBitz34 = new Bitz<kBits34>(pShiftedStreamBuffer, mStreamBufferLength, kBitWidth);
      pBitz35 = new Bitz<kBits35>(pShiftedStreamBuffer, mStreamBufferLength, kBitWidth);
      pBitz37 = new Bitz<kBits37>(pShiftedStreamBuffer, mStreamBufferLength, kBitWidth);
      pBitz40 = new Bitz<kBits40>(pShiftedStreamBuffer, mStreamBufferLength, kBitWidth);
    }
    catch (std::bad_alloc& ba)
    {
      std::cerr << "bad_alloc caught in analyzeBuffers: " << ba.what() << '\n';
      abort();
    }

    pBitz64->display();
    cout << '\n';
    //*
    pBitz26->display();
    pBitz34->display();
    pBitz35->display();
    pBitz37->display();
    pBitz40->display();
    //*/

    delete [] pShiftedStreamBuffer;
    delete pBitz64;
    delete pBitz26;
    delete pBitz34;
    delete pBitz35;
    delete pBitz37;
    delete pBitz40;

    cout << "\n";
  }
	return status_success;
}

exit_status Kepler::displayBuffers()
{
	cout << "Dispaly stuff\n";

	for(int i = 0;i < mStreamBufferLength;++i)
  {
    cout << "|";
    
    for(int j = 0;j < kBitWidth - 1;++j)
    {
      cout << ((j == (kBitWidth/2 - 1)) ? "." : " ");
    }
  }

  cout << '\n';

  for(int i = 0;i < mStreamBufferLength;++i)
  {
    cout << bitset<kBitWidth>(mpStreamBuffer[i]); 
  }

// This could cause problems 
  mpBitz26->display();
  mpBitz34->display();
  mpBitz35->display();
  mpBitz37->display();
  mpBitz40->display();
  mpBitz64->display();
 
  cout << "\n";
/*  
  for(int i = 0;i < mStreamBufferLength;++i)
  {
    cout << bitset<kBitWidth>(mpStreamBuffer[i]); 
  }

  cout << "\n";
//*/

  return status_success;
}


