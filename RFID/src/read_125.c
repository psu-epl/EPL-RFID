#include "head.h"
#include <python2.7/Python.h>

#define SIZE 201
// Compile with gcc run_py.c -lpython2.7
// Requires Python 2.7 libraries to be installed

void* read_125(void* cardNo)
{
  printf("Starting python interpreter\n");
  // This run the python code to read the incoming data on I2C
  Py_Initialize();
  PyObject* obj = PyFile_FromString("i2c.py", "r");
  PyRun_SimpleFileEx(PyFile_AsFile(obj), "i2c.py", 1);
  Py_Finalize();
  printf("Python interpreter finalized\n");

  // Next we read the stored data from the python module
  char str[80];
  FILE* data;
  data = fopen("data.txt", "r");
  if (data == NULL)
  {
    printf("Could not open file\n");
    return 0;
  }

  char line[SIZE];
  line[SIZE] = '\0';
  int length = 0;

  char startBits[] = "000001";
  int len = strlen(startBits);

  fscanf(data, "%200c", line);
  fclose(data);

  char * ret;
  ret = strstr(line, startBits);

  //char cardNo [25];
  //cardNo[24] = '\0';
  strncpy(cardNo, ret+19, 24);

  printf("Tag val: %s\n", cardNo);

  return 0; // Need to return relevant data back to top
}
