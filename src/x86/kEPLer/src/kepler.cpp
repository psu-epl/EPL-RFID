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
Bitz<number_of_bits>::Bitz(int totalBits)
{
  mBitBufferLength = totalBits / number_of_bits;
  mpBitBuffer = new bitset<number_of_bits>[mBitBufferLength]; 
}

template<size_t number_of_bits>
Bitz<number_of_bits>::Bitz(
  uint64_t *pStreamBuffer, 
  int streamBufferLength, 
  int bitWidth
)
{
  shift = (bitWidth - number_of_bits);
  
  int totalBits = streamBufferLength * bitWidth; 
  mBitBufferLength = totalBits / number_of_bits;
  slack = totalBits - (mBitBufferLength * number_of_bits);
  mpBitBuffer = new bitset<number_of_bits>[mBitBufferLength];
  
  uint64_t *pTempStreamBuffer = new uint64_t[streamBufferLength];
  memcpy(pTempStreamBuffer, pStreamBuffer, sizeof(uint64_t) * streamBufferLength);
  convertBuffer(pTempStreamBuffer,streamBufferLength);

  delete pTempStreamBuffer;
}

template<size_t number_of_bits>
Bitz<number_of_bits>::~Bitz()
{
  delete mpBitBuffer;
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
    mpBitBuffer[i] = pStreamBuffer[0] >> shift;
    shiftItAll(pStreamBuffer, streamBufferLength, number_of_bits, shift);
  }

  return status_success;
}

template<size_t number_of_bits>
void Bitz<number_of_bits>::display()
{
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
int Bitz<number_of_bits>::getBitBufferLength() const
{
  return mBitBufferLength;
}

Kepler::Kepler() : 
  mStreamLength(kStreamLength),
  mStreamBufferLength(kStreamLength/2)
{
	mpStreamBuffer = new uint64_t[mStreamBufferLength];
 
  mpBitz26 = NULL;
  mpBitz34 = NULL;
  mpBitz35 = NULL;
  mpBitz37 = NULL;
  mpBitz40 = NULL;
}

Kepler::~Kepler()
{
	delete mpStreamBuffer;
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
  
  int shift = (kBitWidth - number_of_bits);
  
  for(int i = 0;i < buffLength;++i)
  { 
    pBuff[i] = pStreamBuffer[0] >> shift;
    shiftItAll(pStreamBuffer, mStreamBufferLength, number_of_bits, shift);
  }

  delete [] pStreamBuffer;
  
  return status_success;
}

exit_status Kepler::fillBuffers()
{
  uint32_t a = 0x0;
	uint64_t bits = 0x0; 

  for(int i = 0;fin >> hex >> a && i < mStreamLength;++i)
	{
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

//  if(mpBitz26)
//  {
  mpBitz26->display();
  mpBitz34->display();
  mpBitz35->display();
  mpBitz37->display();
  mpBitz40->display();
  cout << "\n";
//  }
//  else
//  {
//    cout << "Preventing null pointer dereference...\n";
//  }

  return status_success;
}

exit_status Kepler::closeFile()
{
	fin.close();
	return status_success;
}

