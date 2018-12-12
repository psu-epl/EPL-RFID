#include "eplurbus.h"

#include <iostream>
#include <fstream>
#include <bitset>
#include <stdio.h>
#include <string.h>

using namespace std;

EPLurbus::EPLurbus() : m_size(streamSize)
{
	m_pRawBitstreamBuffer = new uint64_t[m_size];
//	m_pStringBitstreamBuffer = new string[m_size];

	m_size26 = (m_size * bitwidth) / 26;
	m_pBuff26 = new uint64_t[m_size26];   
	
	m_size34 = (m_size * bitwidth) / 34;
	m_pBuff34 = new uint64_t[m_size34];   
	
	m_size35 = (m_size * bitwidth) / 35;
	m_pBuff35 = new uint64_t[m_size35];   
	
	m_size37 = (m_size * bitwidth) / 37;
	m_pBuff37 = new uint64_t[m_size37];   
	
	m_size40 = (m_size * bitwidth) / 40;
	m_pBuff40 = new uint64_t[m_size40];
}

EPLurbus::~EPLurbus()
{
	delete m_pRawBitstreamBuffer;
//	delete m_pStringBitstreamBuffer;
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

exit_status EPLurbus::convertBuffer()
{
  uint64_t temp = 0x0;
  const int validBits = 27;

  if(!m_pRawBitstreamBuffer)
  {
    return status_failure;
  }
  uint64_t *workingRawStreamBuffer = new uint64_t[m_size];
  memcpy(workingRawStreamBuffer, m_pRawBitstreamBuffer, sizeof(uint64_t)*m_size);

  cout << "raw:\n" << std::bitset<bitwidth>(m_pRawBitstreamBuffer[0]) << '\n';
  temp = m_pRawBitstreamBuffer[0] >> (bitwidth - validBits);
	
  cout << "temp:\n" << std::bitset<validBits>(temp) << '\n';

  return status_success;
}

exit_status EPLurbus::fillBuffers()
{
  uint32_t a = 0x0;
	uint64_t bits = 0x0; 
	
//	while(fin >> hex >> a)   /// Notice how the loop is done.
	for(int i = 0;fin >> hex >> a && i < m_size;++i)
	{
    if(i % 2 == 0)
    {
      bits = ((uint64_t) a) << 32;  
		  //cout << std::bitset< bitwidth >(bits) << '\n';
    }
    else
    {
      bits |= a;
		  //cout << std::bitset< bitwidth >(bits) << '\n';
		  m_pRawBitstreamBuffer[i/2] = bits;
		  cout << std::bitset<bitwidth>(m_pRawBitstreamBuffer[i/2]) << '\n';
      bits = 0x0;
    }

//		m_pRawBitstreamBuffer[i] = bits;
//		m_pStringBitstreamBuffer[i] = new string (std::bitset< bitwidth >( a ).to_string());
		//cout << m_pStringBitstreamBuffer[i] << '\n';
		//cout << std::bitset< bitwidth >( a ) << '\n';
//		cout << std::bitset< bitwidth >( m_pRawBitstreamBuffer[i] ) << '\n';
//    bits = 0x0;
	}
  convertBuffer();
	return status_success;
}

exit_status EPLurbus::displayBuffers()
{
	cout << "Dispaly stuff\n";
	//for(int i = 0;i < m_size; ++i)
	//{
		
	//}
}

exit_status EPLurbus::closeFile()
{
	fin.close();
	return status_success;
}
