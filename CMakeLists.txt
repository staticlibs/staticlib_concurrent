# Copyright 2017, alex at staticlibs.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 2.8.12 )

project ( staticlib_concurrent CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 1.0.3 )
set ( ${PROJECT_NAME}_DESCRIPTION "Staticlibs concurrent library" )
set ( ${PROJECT_NAME}_URL https://github.com/staticlibs/staticlib_concurrent )

# docs
option ( ${PROJECT_NAME}_ENABLE_DOCS "Generate doxyfile and exit build" OFF )
if ( ${PROJECT_NAME}_ENABLE_DOCS )
    configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
    return ( )
endif ( )

# target
file ( GLOB_RECURSE ${PROJECT_NAME}_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp )
source_group ( "include" FILES ${${PROJECT_NAME}_HEADERS} )
add_custom_target ( ${PROJECT_NAME} SOURCES ${${PROJECT_NAME}_HEADERS} )

# pkg-config
set ( ${PROJECT_NAME}_PC_CFLAGS "-I${CMAKE_CURRENT_LIST_DIR}/include" )
if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
    set ( ${PROJECT_NAME}_PC_LIBS_PRIVATE "-lpthread" )
endif ( )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/pkg-config.in 
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig/${PROJECT_NAME}.pc )
