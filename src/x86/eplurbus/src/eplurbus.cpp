#include "eplurbus.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>

using namespace std;

EPLurbus::EPLurbus() : m_size(streamSize)
{
	m_pRawBitstreamBuffer = new uint64_t[m_size];
  int totalBits = m_size / 2 * bitwidth; 
	m_size26 = totalBits / bits26;
	m_pBuff26 = new bitset<bits26>[m_size26];   
	
	m_size34 = totalBits / bits34;
	m_pBuff34 = new bitset<bits34>[m_size34];   
	
	m_size35 = totalBits / bits35;
	m_pBuff35 = new bitset<bits35>[m_size35];   
	
	m_size37 = totalBits / bits37;
	m_pBuff37 = new bitset<bits37>[m_size37];   
	
	m_size40 = totalBits / bits40;
	m_pBuff40 = new bitset<bits40>[m_size40];
}

EPLurbus::~EPLurbus()
{
	delete m_pRawBitstreamBuffer;
	delete m_pBuff26;
	delete m_pBuff34;
	delete m_pBuff35;
	delete m_pBuff37;
	delete m_pBuff40;
}

exit_status EPLurbus::openFile(string filename)
{
	fin.open(filename);
	if(fin.fail())
	{
		cout << "Open file failure: " << filename << "\n";
		return status_failure;
	}
	return status_success;
}

exit_status EPLurbus::shiftLeft()
{
  /*
  unsigned char bits1 = 0, bits2 = 0;
  for(i = len-1; i >= 0; --i) {
    bits2 = array[i] & 0x07;
    array[i] >>= 3;
    array[i] |= bits1 << 5;
    bits1 = bits2;
  }
  //*/
  return status_success;
}

void EPLurbus::shiftItAll(uint64_t *pBuff, size_t n)
{
    for(int j = 0; j < m_size - 1;++j)
    {
      pBuff[j] = (pBuff[j] << n) | (pBuff[j+1] >> n);
    }
}

template<size_t number_of_bits>
exit_status EPLurbus::convertBuffer2()
{
  bitset<number_of_bits> validBits = 0x0;

  if(!m_pRawBitstreamBuffer)
  {
    return status_failure;
  }

  uint64_t *workingRawStreamBuffer = new uint64_t[m_size];
  memcpy(workingRawStreamBuffer, m_pRawBitstreamBuffer, sizeof(uint64_t)*m_size);
  int shift = 0;
  for(int i = 0;i < m_size26;++i)
  { 
    //cout << "raw:\n" << std::bitset<bitwidth>(workingRawStreamBuffer[i]) << '\n';
    shift = (bitwidth - number_of_bits);
    m_pBuff26[i] = workingRawStreamBuffer[0] >> shift;
    //cout << "buff26[" << i << "]: " << m_pBuff26[i] << '\n';
    cout << m_pBuff26[i];
    //cout << m_pBuff26[i] << " ";

    shiftItAll(workingRawStreamBuffer,number_of_bits);

  }

  delete workingRawStreamBuffer;
  
  return status_success;
}


exit_status EPLurbus::convertBuffer()
{
  uint64_t temp = 0x0;
  const int validBits = 26;

  if(!m_pRawBitstreamBuffer)
  {
    return status_failure;
  }

  uint64_t *workingRawStreamBuffer = new uint64_t[m_size];
  memcpy(workingRawStreamBuffer, m_pRawBitstreamBuffer, sizeof(uint64_t)*m_size);

  cout << "raw:\n" << std::bitset<bitwidth>(m_pRawBitstreamBuffer[0]) << '\n';
  temp = m_pRawBitstreamBuffer[0] >> (bitwidth - validBits);
	
  cout << "temp:\n" << std::bitset<validBits>(temp) << '\n';

  delete workingRawStreamBuffer;
  return status_success;
}

exit_status EPLurbus::fillBuffers()
{
  uint32_t a = 0x0;
	uint64_t bits = 0x0; 
	
	for(int i = 0;fin >> hex >> a && i < m_size;++i)
	{
    if(i % 2 == 0)
    {
      bits = ((uint64_t) a) << 32;  
    }
    else
    {
      bits |= a;
		  m_pRawBitstreamBuffer[i/2] = bits;
		  cout << std::bitset<bitwidth>(m_pRawBitstreamBuffer[i/2]);
		  //cout << std::bitset<bitwidth>(m_pRawBitstreamBuffer[i/2]) << " ";
		  //cout << std::bitset<bitwidth>(m_pRawBitstreamBuffer[i/2]) << '\n';
      bits = 0x0;
    }
	}
  cout << "\n-----------------------\n";
  convertBuffer2<26>();
	return status_success;
}

exit_status EPLurbus::displayBuffers()
{
//	cout << "Dispaly stuff\n";
	for(int i = 0;i < 22; ++i)
	{
    cout << "*";	
	}
    cout << '\n';	
}

exit_status EPLurbus::closeFile()
{
	fin.close();
	return status_success;
}
