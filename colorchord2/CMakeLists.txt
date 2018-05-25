cmake_minimum_required(VERSION 3.10)
project(colorchord2 C)

set(CMAKE_C_STANDARD 90)

add_executable(colorchord2 main.c

        os_generic.c  dft.c decompose.c
        filter.c color.c notefinder.c util.c
        outdrivers.c DrawFunctions.c parameters.c
        chash.c WinDriver.c sound.c sound_null.c
        sound_win.c OutputVoronoi.c
        OutputProminent.c DisplayArray.c
        OutputLinear.c DisplayPie.c
        DisplayNetwork.c hook.c
        RecorderPlugin.c
        ../embeddedcommon/DFT32.c
        OutputCells.c configs.c hidapi.c
        DisplayHIDAPI.c

        #Sounds
        sound.c sound_alsa.c sound_pulse.c sound_null.c

        #OUTS
        OutputVoronoi.c DisplayArray.c OutputLinear.c DisplayPie.c DisplayNetwork.c DisplayUSB2812.c DisplayDMX.c OutputProminent.c RecorderPlugin.c DisplayHIDAPI.c hidapi.c OutputCells.c DisplaySHM.c DisplayFileWrite.c

        #RawDraw
        DrawFunctions.c XDriver.c

        )