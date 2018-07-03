#include "head.h"

int init() // Initialises all needed utilities
{
  // Start GPIO init
  if (gpioInitialise() < 0)
  {
    printf("pigpio init failed\n");
    return -1;
  }

  // Set GPIO modes here
  //gpioSetMode(4, PI_OUTPUT); // GPIO 4 as logic output for machine state
  //gpioSetMode(18, PI_INPUT); // GPIO 18 as input from 125 KHz reader

}

void cleanup() // Clean up after ourselves
{
  gpioTerminate();
}

int read_station_info(char* station_id)
{
  FILE* fp;
  fp = fopen("station_info.txt", "r");
  // If there is no Station ID, prompt the user to make one
  if (fp == NULL)
  {
    printf("\nStation ID not found. Would you like to create one?\n[y/n]\n");
    char temp;
    scanf("%1c", &temp);
    if (temp == 'y' || temp == 'Y')
    {
      fp = fopen("station_info.txt", "w");
      printf("\nPlease enter a unique Station ID. No spaces. Max 100 characters.\n");
      scanf("%100s", station_id);
      printf("\nStation ID: %s\n", station_id);
      fprintf(fp, station_id); // Store station_id id to station_id_info.txt
      fclose(fp);
      printf("Station ID has been successfully saved!\n");
    }
    else
    {
      printf("\nCannot run program without Station ID. Exiting...\n");
      return 1;
    }
  }
  else
  {
    // Read Station ID from file
    fscanf(fp, "%s", station_id);
    printf("Station ID is: %s\n", station_id);
    fclose(fp);
  }
}


