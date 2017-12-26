# `libraspivid`

### Introduction

A collection of C++11 classes that implement Raspberry Pi Camera functionality. Much of the code has been adapted 
directly from [Raspberry Pi's `raspicam` implementation](https://github.com/raspberrypi/userland/tree/master/host_applications/linux/apps/raspicam)
with some modifications.

### Features

- C++11 class wrappers for many Raspberry Pi Camera components
- Automatic component initialization/destruction
- Callback wrapper class for easy callback implementation

### Usage

The included `CMakeLists.txt` file exports the `raspivid` static library and a `${LIBRASPIVID_INCLUDE_DIRS}` environment variable.
You can setup a CMake project directory structure like this one:

```
/project_root
   CMakeLists.txt
   /libraspivid
   /src
      my_project.cpp   
```

Your CmakeLists.txt file should appear as follows:

```
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
project(libraspivid_example_project)
add_subdirectory(libraspivid)
include_directories(${LIBRASPIVID_INCLUDE_DIRS})
add_executable(my_project ./src/my_project.cpp)
target_link_libraries(my_project raspivid)
```

Be sure that your `my_project.cpp` includes...

```C++
#include "RaspiVid.h"
using namespace raspivid;
```

### LICENSE

This code is derived from `raspicam`, and thus retains its original license:

```
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, James Hughes
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```


