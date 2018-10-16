cmake_minimum_required(VERSION 3.10)
project(colorchord2 C)
set(CMAKE_C_STANDARD 90)


if(UNIX)
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

else() #windows
    add_executable(colorchord2
            os_generic.c main.c  dft.c decompose.c
            filter.c color.c notefinder.c util.c outdrivers.c
            DrawFunctions.c parameters.c chash.c WinDriver.c sound.c
            sound_null.c sound_win.c OutputVoronoi.c OutputProminent.c
            DisplayArray.c OutputLinear.c DisplayPie.c DisplayNetwork.c
            hook.c RecorderPlugin.c ../embeddedcommon/DFT32.c OutputCells.c
            configs.c hidapi.c DisplayHIDAPI.c

            #main.c os_generic.c dft.c decompose.c filter.c color.c notefinder.c util.c outdrivers.c parameters.c chash.c hook.c ../embeddedcommon/DFT32.c configs.c
            #os_generic.c WinDriver.c

            #OUTS
            #OutputVoronoi.c DisplayArray.c OutputLinear.c DisplayPie.c DisplayNetwork.c OutputProminent.c RecorderPlugin.c DisplayHIDAPI.c hidapi.c OutputCells.c
            #OutputRaw.c

            #sounds
            #sound.c sound_null.c sound_win.c

            # not in windows:
            # DisplayUSB2812.c DisplayDMX.c DisplaySHM.c DisplayFileWriter.c DrawFunctions.c XDriver.c

            )
    target_link_libraries(colorchord2 ws2_32 winmm gdi32 setupapi)
endif()

