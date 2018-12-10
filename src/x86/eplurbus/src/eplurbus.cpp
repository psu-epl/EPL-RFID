#include "eplurbus.h"

#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

EPLurbus::EPLurbus() : m_size(streamSize)
{
	m_pRawBitstreamBuffer = new uint32_t[m_size];
//	m_pStringBitstreamBuffer = new string[m_size];

	m_size26 = (m_size * bitwidth) / 26;
	m_pBuff26 = new uint32_t[m_size26];   
	
	m_size34 = (m_size * bitwidth) / 34;
	m_pBuff34 = new uint32_t[m_size34];   
	
	m_size35 = (m_size * bitwidth) / 35;
	m_pBuff35 = new uint32_t[m_size35];   
	
	m_size37 = (m_size * bitwidth) / 37;
	m_pBuff37 = new uint32_t[m_size37];   
	
	m_size40 = (m_size * bitwidth) / 40;
	m_pBuff40 = new uint32_t[m_size40];
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

exit_status EPLurbus::fillBuffers()
{
	unsigned int a;
	
//	while(fin >> hex >> a)   /// Notice how the loop is done.
	for(int i = 0;fin >> hex >> a && i < m_size;++i)
	{
		m_pRawBitstreamBuffer[i] = a;
//		m_pStringBitstreamBuffer[i] = new string (std::bitset< bitwidth >( a ).to_string());
		//cout << m_pStringBitstreamBuffer[i] << '\n';
		//cout << std::bitset< bitwidth >( a ) << '\n';
		cout << std::bitset< bitwidth >( m_pRawBitstreamBuffer[i] ) << '\n';
	}

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
