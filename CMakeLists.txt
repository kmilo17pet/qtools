# qLibs CMakeLists.txt file
# Copyright (C) 2012 Eng. Juan Camilo Gómez Cadavid MSc. All Rights Reserved.
#
# To make use of the qLibs libraries on your solution just add the following
# directives to the top-level CMakeLists.txt file :
#
#     add_subdirectory( <relative/path/to/qlibs> )
#     target_link_libraries( ${PROJECT_NAME} qlibs )
#
# This file is part of the qLibs distribution.

cmake_minimum_required( VERSION 3.2 )
project( qlibs
         VERSION 1.2.9
         DESCRIPTION "A collection of useful libraries for embedded systems"
         LANGUAGES C
       )

add_library( ${PROJECT_NAME}
             qbitfield.c
             qcrc.c
             qffmath.c
             qfis.c
             qfp16.c
             qltisys.c
             qnuma.c
             qpid.c
             qrms.c
             qssmoother.c
             qtdl.c
             qtypegeneric.c
             qvfloat.c
             qinterp1.c
           )
target_include_directories( ${PROJECT_NAME} PUBLIC include )
