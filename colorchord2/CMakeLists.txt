cmake_minimum_required(VERSION 3.10)
project(colorchord2 C)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_C_STANDARD 90)


IF(UNIX)
    add_executable(colorchord2
            main.c  os_generic.c dft.c decompose.c filter.c color.c notefinder.c util.c outdrivers.c parameters.c chash.c hook.c ../embeddedcommon/DFT32.c configs.c

            #OUTS
            OutputVoronoi.c DisplayArray.c OutputLinear.c DisplayPie.c DisplayNetwork.c DisplayUSB2812.c DisplayDMX.c OutputProminent.c RecorderPlugin.c DisplayHIDAPI.c hidapi.c OutputCells.c DisplaySHM.c DisplayFileWrite.c
            OutputRaw.c

            #RawDraw
            DrawFunctions.c XDriver.c


            #Sounds
            sound.c sound_alsa.c sound_pulse.c sound_null.c

    )

    target_link_libraries(colorchord2
            #RawDrawLibs
            -lX11 -lm -lpthread -lXinerama -lXext
            #LDLIBS
            -lpthread -lasound -lm -lpulse-simple -lpulse -ludev -lrt
            #extraLibs
            -lusb-1.0
    )

ELSE(UNIX)  #windows
    add_executable(colorchord2
            main.c  os_generic.c dft.c decompose.c filter.c color.c notefinder.c util.c outdrivers.c parameters.c chash.c hook.c ../embeddedcommon/DFT32.c configs.c

            #OUTS
            OutputVoronoi.c DisplayArray.c OutputLinear.c DisplayPie.c DisplayNetwork.c DisplayUSB2812.c DisplayDMX.c OutputProminent.c RecorderPlugin.c DisplayHIDAPI.c hidapi.c OutputCells.c DisplaySHM.c DisplayFileWrite.c
            OutputRaw.c

            #RawDraw
            DrawFunctions.c XDriver.c


            os_generic.c WinDriver.c

            #sounds
            sound.c sound_null.c sound_win.c
    )
ENDIF()

