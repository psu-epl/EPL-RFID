#include "kepler.h"

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>

using namespace std;

Kepler::Kepler() : 
  mStreamLength(streamLength),
  mStreamBufferLength(streamLength/2)
{
	mpStreamBuffer = new uint64_t[mStreamBufferLength];
  int totalBits = mStreamBufferLength * bitwidth; 
	
  mBuff26Length = totalBits / bits26;
	mpBuff26 = new bitset<bits26>[mBuff26Length];
  //memset(mpBuff26,0,mpBuff26->size());  
  mBuff34Length = totalBits / bits34;
	mpBuff34 = new bitset<bits34>[mBuff34Length];
/*  
	m_size34 = totalBits / bits34;
	m_pBuff34 = new bitset<bits34>[m_size34];   
	
	m_size35 = totalBits / bits35;
	m_pBuff35 = new bitset<bits35>[m_size35];   
	
	m_size37 = totalBits / bits37;
	m_pBuff37 = new bitset<bits37>[m_size37];   
	
	m_size40 = totalBits / bits40;
	m_pBuff40 = new bitset<bits40>[m_size40];
//*/
}

Kepler::~Kepler()
{
	delete mpStreamBuffer;
	delete mpBuff26;
	delete mpBuff34;
//	delete m_pBuff35;
//	delete m_pBuff37;
//	delete m_pBuff40;
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

/*
void Kepler::printShift()
{
	for(int i = 0;i < mBuff26Length;++i)
  {
    cout << "|";
    for(int j = 0;j < bits26 - 1;++j)
    {
      cout << ' ';
    }
  }
  cout << '\n';
}

void Kepler::printStreamBuffer(uint64_t *pBuff)
{
  for(int j = 0; j < mStreamBufferLength;++j)
  {
    cout << bitset<bitwidth>(pBuff[j]);
  }
}
//*/

void Kepler::shiftItAll(uint64_t *pBuff, size_t n, int shift)
{
  (void)shift;
  for(int j = 0; j < mStreamBufferLength;++j)
  {
    pBuff[j] = (pBuff[j] << n) | (pBuff[j+1] >> shift);
  }
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
  
  int shift = (bitwidth - number_of_bits);
  
  for(int i = 0;i < buffLength;++i)
  { 
    pBuff[i] = pStreamBuffer[0] >> shift;
    shiftItAll(pStreamBuffer, number_of_bits, shift);
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

  convertBuffer<bits26>(mpBuff26, mBuff26Length);
  convertBuffer<bits34>(mpBuff34, mBuff34Length);

	return status_success;
}

exit_status Kepler::displayBuffers()
{
	cout << "Dispaly stuff\n";

	for(int i = 0;i < mStreamBufferLength;++i)
  {
    cout << "|";
    
    for(int j = 0;j < bitwidth - 1;++j)
    {
      cout << ((j == (bitwidth/2 - 1)) ? "." : " ");
    }
  }

  cout << '\n';

  for(int i = 0;i < mStreamBufferLength;++i)
  {
    cout << bitset<bitwidth>(mpStreamBuffer[i]); 
  }

  cout << '\n';

	for(int i = 0;i < mBuff26Length;++i)
  {
    cout << "|";
    for(int j = 0;j < bits26 - 1;++j)
    {
      cout << ' ';
    }
  }

  cout << '\n';

  for(int i = 0;i < mBuff26Length;++i)
  {
    cout << bitset<bits26>(mpBuff26[i]); 
  }

//* TODO:make this less gross
	for(int i = 0;i < 22; ++i)
	{
    cout << "*";	
	}
//*/
  cout << '\n';	

  return status_success;
}

exit_status Kepler::closeFile()
{
	fin.close();
	return status_success;
}
