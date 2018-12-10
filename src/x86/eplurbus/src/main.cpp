#include "eplurbus.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	exit_status status = status_success;
	
	EPLurbus *eplurbus = new EPLurbus();
	
	status = eplurbus->openFile("data/c_rfid.txt");
	if(status)
	{
		delete eplurbus;
		return 1;
	}

	eplurbus->fillBuffers();
	eplurbus->displayBuffers();
	eplurbus->closeFile();
	
	delete eplurbus;

	return 0;
}
