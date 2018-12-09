#include "eplurbus.h"

#include <iostream>
#include <fstream>

static int EPLurbus::size	= 100;

EPLurbus::EPLurbus()
{
	size26 = (size * 64) / 26;
	pBuff26 = new uint64_t[size26];   
	
	size34 = (size * 64) / 34;
	pBuff34 = new uint64_t[size34];   
	
	size35 = (size * 64) / 35;
	pBuff35 = new uint64_t[size35];   
	
	size37 = (size * 64) / 37;
	pBuff37 = new uint64_t[size37];   
	
	size40 = (size * 64) / 40;
	pBuff40 = new uint64_t[size40];   
}


EPLurbus::~EPLurbus()
{


}
