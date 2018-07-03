all: main reset

main: main.o funcs.o read_125.o
	gcc -g main.o funcs.o read_125.o -o station -lpigpio -lrt -lpthread -lpython2.7 -lwiringPi -lbcm2835

main.o: main.c
	gcc -g -c main.c -lpigpio -lrt -lpthread -lwiringPi -lbcm2835

funcs.o: funcs.c
	gcc -g -c funcs.c -lpigpio -lrt

read_125.o: read_125.c
	gcc -g -c read_125.c -lpython2.7

read_13.o: read_13.c value.c config.c rfid.c re522.c
	gcc -g -c -Wall -Wextra read_13.c value.c config.c rfid.c re522.c -lwiringPi -lbcm2835

reset: station_reset.o
	gcc -g station_reset.o -o reset

station_reset.o: station_reset.c
	gcc -g -c station_reset.c

clean:
	rm -f station main.o funcs.o read_125.o station_info.txt reset station_reset.o
