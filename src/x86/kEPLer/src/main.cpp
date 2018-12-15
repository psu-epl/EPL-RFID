#include "kepler.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
	exit_status status = status_success;
	
	Kepler *kepler = new Kepler();
	
	status = kepler->openFile("data/c_rfid.txt");
	if(status)
	{
		delete kepler;
		return 1;
	}

	kepler->fillBuffers();
	kepler->displayBuffers();
	kepler->closeFile();
	
	delete kepler;

	return 0;
}
