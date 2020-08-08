# Ultrasound Thermometry X(correlation) tool

## Overview

This repository hosts a reference C++ implementation for so called cross-correlation ultrasonic thermometry method.

[Ultrasonic/ultrasound thermometry](https://imsysinc.com/Knowledgebase/ultratherm.htm) (and research on this specific algorithm) was the subject of my [bachelor study](https://elib.spbstu.ru/dl/2/v18-2590.pdf/info).
The implemented algorithm is described in the ['Two-dimensional temperature estimation using diagnostic ultrasound
' paper by Simon et al.](https://ieeexplore.ieee.org/document/710592) with some modifications applied by me.
This implementation was built during my post-bachelor research work in the university. Unfortunately, I did not manage
to publish any papers describing either research results or this particular software. However, if you are highly interested
int this work, please contact me via my GitHub profile or email. 

## Disclaimer

The **only** purposes of this repo:
* to keep somewhere results of my work (as some kind of museum value)
* to provide the reference implementation of algorithm for the interested ones

Known issues:
* Code may be a little bit messy since I wrote this thing years ago and, frankly speaking, some solutions in it seem..
_strange_ to me nowadays. 

## Software description

The processing pipeline is the following:
1. Program reads a series of raw signals obtain from the ultrasound sensor (sample data can be found at `data/h_6mm`).
Raw data is presented as 2D int16 arrays written to a binary file with no format or processing.
2. Then, data is processed and thermal shift at each moment of time is computed.
3. Thermal shift for each moment of time are written to a [Tecplot PLT binary file](https://www.tecplot.com/2016/09/16/tecplot-data-file-types-dat-plt-szplt/)

After that, the actual temperature field may be found by multiplying thermal shift by a calibration coefficient which
highly depends on material. Calibration procedure is out of scope of this code.

## Technical notes

1. This tool was designed to be quite fast, but I am not sure if I've succeed at this exercise.
2. Code is meant to be simple and portable. Because of that I implemented my own [DSP micro-library](https://github.com/dev0x13/dsperado) and
avoided using third-party libraries which requires separate compilation.
3. The tool was initially developed for Windows and was being built only via Visual Studio, but at the time
I am writing this, I've changed the build system to CMake. Build was tested on Ubuntu 20.04 with GCC 9.3 and Windows 10 with MSVC v142.

## Running the tool

cmake --build . --parallel --config Release

... 
