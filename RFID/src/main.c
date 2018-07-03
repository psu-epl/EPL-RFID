#include "head.h"
#include "main.h"

int main(int argc, char *argv[])
{
  int station_control = 1; // Var to terminate program
  char* station_id = (char*) malloc(sizeof(char)*100);
  int station_state = 0; // Stations current state, 1 = ON, 0 = OFF
  pthread_t thread_id1;
  pthread_t thread_id2;

  printf("Initializing settings\n");
  init();
  init_13();

  printf("Finished initializing\n");

  read_station_info(station_id); // Get station info from file or create it

  while (station_control == 1) // Exiting this loop will end the program
  {
    void* cardNo125;
    void* cardNo13;

    printf("Starting a 125 read\n");

    pthread_create(&thread_id1, NULL, &read_125, &cardNo125);
    pthread_create(&thread_id2, NULL, &send_card_no, &cardNo13);
    printf("Threads created\n");

    printf("Start a 13.5 read\n");

    while(1);

    station_control = 0; // Turn off for testing purposes
  }

  cleanup(); // Release resources
  return 0;
}
