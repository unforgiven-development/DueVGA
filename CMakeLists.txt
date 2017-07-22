cmake_minimum_required(VERSION 3.7)
project(DueVGA)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        examples/NTSC/DrawingTestNTSC/DrawingTestNTSC.ino
        examples/PAL/DrawingTestPAL/DrawingTestPAL.ino
        examples/PAL/NewtonPAL/NewtonPAL.ino
        examples/VGA/COLOR/DrawingTest/DrawingTest.ino
        examples/VGA/COLOR/HelloWorldColor/HelloWorldColor.ino
        examples/VGA/COLOR/Mandelbrot/Mandelbrot.ino
        examples/VGA/COLOR/Newton/Newton.ino
        examples/VGA/COLOR/PatternColor/PatternColor.ino
        examples/VGA/COLOR/Ripple/Ripple.ino
        examples/VGA/MONO/ASCIITableVGA/ASCIITableVGA.ino
        examples/VGA/MONO/FourierSeries/FourierSeries.ino
        examples/VGA/MONO/HelloWorld/HelloWorld.ino
        examples/VGA/MONO/Multiply/Multiply.ino
        examples/VGA/MONO/Pattern/Pattern.ino
        examples/VGA/MONO/Rossler/Rossler.ino
        extras/GPS/earth.ino
        extras/GPS/GPS.ino
        extras/SDCard/Anim/Anim.ino
        extras/SDCard/anim.dat
        extras/SDCard/makeanim.py
        extras/Waterfall/waterfall/waterfall.ino
        src/crntsc.h
        src/crpal.h
        src/font8x8.cpp
        src/graphics.cpp
        src/text.cpp
        src/VGA.cpp
        src/VGA.h
        keywords.txt
        library.properties)

add_executable(DueVGA ${SOURCE_FILES})
