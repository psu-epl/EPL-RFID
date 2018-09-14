
This is the EPL RFID ESP-IDF template app
========================================

This is a template application to be used with `Espressif IoT Development Framework`_ (ESP-IDF). 

Please check ESP-IDF docs for getting started instructions.

Code in this repository is Copyright (C) 2016 Espressif Systems, licensed under the Apache License 2.0 as described in the file LICENSE.

.. _Espressif IoT Development Framework: https://github.com/espressif/esp-idf

Usage:

Copy the template using "cp -r app_template app_somename"
Edit the Makefile on the line that says:

PROJECT_NAME := app_template

Substitute app_template with app_somename

For a faster build "make -j4" is fun but will rebuild the entire app (TODO: fix this bug)
Use make flash to only build your main files.

Note: This template is a WIP and will improve as the build system improves. These 
improvements in the build system will be useful for other EPL projects.

