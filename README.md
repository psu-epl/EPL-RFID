# EPL-RFID
RFID system for controlling access to EPL machines

This repo started as copies of two separate Capstone projects in 2018: the CS
backend Capstone called LUCCA (Lab Usage Controller for Centralized Administration)
and the ECE Capstone called EPL RFID Capstone Project.

Here are links to the previous repos:

### LUCCA:
#### Repo: https://github.com/LUCCA-Capstone/LUCCA
#### Wiki: https://github.com/LUCCA-Capstone/LUCCA/wiki

### EPL RFID:
#### Repo: https://github.com/Crimson89/RFID-Capstone
#### Wiki: https://github.com/Crimson89/RFID-Capstone/wiki

# Building ESP-IDF toolchain and environment
From a linux console type the following commands:   

git clone https://github.com/psu-epl/EPL-RFID.git  
cd EPL-RFID  
git submodule update --init  

Then follow "the Setting Up ESP-IDF" directions here: https://github.com/espressif/esp-idf  

This involves lots of building packages from source. You likely don't want to do this very many times.

make sure you edit your PATH variable in your ~/.bashrc to something similar to:

export IDF_PATH=~/path/to/submodule/esp-idf
export PATH=$PATH:/home/chad/path/to/binaries/xtensa-esp32-elf/bin:/home/chad/path/to/binaries/crosstool-NG/builds/xtensa-esp32-elf/bin/

Bad things happen when you are following the wrong path...

You can verify your path by typing "echo $PATH"

Code victoriously brave warrior!
