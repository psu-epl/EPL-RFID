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
  cout << "\nControl buffers: \n\n";
  cout << "Bitwidth\n";
  cout << "|\n";
  cout << "v\n";
	kepler->displayBuffers();
  cout << "\n\n";
  cout << "Going down the rabbit hole: \n";
  kepler->analyzeBuffer<kBits26>();
  kepler->analyzeBuffer<kBits34>();
  kepler->analyzeBuffer<kBits35>();
  kepler->analyzeBuffer<kBits37>();
  kepler->analyzeBuffer<kBits40>();
  kepler->analyzeBuffer<kBits40>();
  cout << "\n\n";
	kepler->closeFile();
	
	delete kepler;

	return 0;
}
