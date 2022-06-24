# Issue recreation

During the quest of testing the parallel execution policies for C++17 Std Algorithms, a segmentation fault appears during the cleanup phase after having executed correctly the main program.

## Requirements
The issue has been debugged so far in an environment using the following modules, which must be available when during the building phase:

* gcc/11.2.0
* nvhpc/22.3 (using CUDA 11.6)   
* boost/1.72.0  
* eigen/3.4.0

Clone the repo:
```
git clone git@github.com:ugGit/actscore-issue-recreation.git
```

## Information about the setup
The code has been tested on a Nvidia GeForce RTX 2080.

This project contains the minimal required modules and code to recreate the issue encountered.

The code is only an extract of a more complex project. To follow the same procedure as later for production required, only one part will be compiled with the nvc++ compiler, whereas gcc is used for the rest. The dynamic selection of compiler within CMake is not possible. To work around this issue, a switch is built in to the auxiliary script `nvc++_p` which defines the compiler per compilation directory.
Another reason for the auxiliary script are some flags (e.g. `-Wno-unused-local-typedefs`) that CMake adds to the compilation process which are invalid for nvc++. Therefore, they get stripped from the command chain by the auxiliary script.

## Configuration
To run the nvc++ compiler with the desired gcc version, a localrc file must be generated. This can be done by executing a script provided by nvhpc:

```
makelocalrc -gcc PATH_TO_GCC -gpp PATH_TO_G++ -x -d PATH_TO_LOCALRC_DIR
```

Then, the auxiliary script `nvc++_p` must be updated such that the variable `LOCALRC` points to PATH_TO_LOCALRC_DIR defined in the above command. E.g.:
```
LOCALRC="/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/localrc_gcc112"
```

## Running the code
The current state should allow compiling and linking the program successfully, while there should be a segfault at the end of the execution. Therefore, pass the auxiliary script `nvc++_p` as compiler during the configuration of CMake as follows:
```
cmake -S . -B build -DCMAKE_CXX_COMPILER=$PWD/nvc++_p
```

Then build and execute the program:
```
cmake --build build/
./build/TestActsCore
```

Which should yield the following output:
```
z
-----------
0.840188
0.394383
1.23457
Segmentation fault (core dumped)
```

## Observations
The following list resumes the changes made that allow to run the program successful:
* do not link the ActsCore library 
* remove the vector of string `vec` of the program
* change the execution policy to `par` or `par_unseq`
* building for `multicore` (and removing `-gpu=managed:intercept`)

### Trying to compile ActsCore with nvc++
We tried to compile [ActsCore](https://github.com/acts-project/acts) using nvc++ to exclude possible link incompatibility issues, but did not succeed. 
The compilation fails on some meta programming.
 More information and reproduction steps can be found here: https://github.com/ugGit/acts.

### Using gcc/10.1.0
The suggestion to use another gcc compiler version results in the same segmentation fault for the `seq` execution policy.

### Removing the `-gpu=managed:intercept` flag
When removing the flag, the `par` and `par_unseq` execution policies result in a `free(): invalid pointer` error instead of a segfault.

### Backtrace of the segfault
A first analysis with the debugger shows the following backtrace when the segmentation fault occurs:

```
Thread 1 "TestActsCore" received signal SIGSEGV, Segmentation fault.
0x00000000004133cc in std::__cxx11::basic_string::_M_data ()
    at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/basic_string.h:195
195	      { return _M_dataplus._M_p; }
(gdb) backtrace
#0  0x00000000004133cc in std::__cxx11::basic_string::_M_data ()
    at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/basic_string.h:195
#1  0x0000000000413555 in std::__cxx11::basic_string::_M_is_local ()
    at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/basic_string.h:230
#2  0x0000000000413595 in std::__cxx11::basic_string::_M_dispose ()
    at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/basic_string.h:239
#3  0x0000000000413b95 in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > () at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/basic_string.h:671
#4  0x0000000000417255 in std::_Destroy (__pointer=0x7fff92008e80)
    at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/stl_construct.h:140
#5  0x00000000004132f2 in __destroy (__first=0x7fff92008e80, __last=0x7fff92008fa0)
    at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/stl_construct.h:152
#6  0x000000000041722d in std::_Destroy (__first=0x7fff92008e80, __last=0x7fff92008fa0)
    at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/stl_construct.h:184
#7  0x00000000004172b1 in std::_Destroy (__first=0x7fff92008e80, __last=0x7fff92008fa0, 
    _T55_57457=...) at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/alloc_traits.h:746
#8  0x00000000004122c1 in std::vector::~vector ()
    at /bld4/opt/gcc/11.2.0/include/c++/11.2.0/bits/stl_vector.h:680
#9  0x00007fffeb9e005a in __cxa_finalize () from /lib64/libc.so.6
#10 0x00007ffff616a913 in __do_global_dtors_aux ()
   from /home/nwachuch/bld6/nvcpp-tests/tmp33/build/_deps/acts-build/lib64/libActsCore.so
#11 0x00007fffffffd810 in ?? ()
#12 0x00007ffff7deb08a in _dl_fini () from /lib64/ld-linux-x86-64.so.2
```

Based on the error log and the debugger output above, we assume the error is caused by the destruction of a `string` within a `vector`. This can also be confirmed, by commenting the creation of the vector of strings (`std::vector<std::string> vec {"z", "y", "x"};`). If the project is rebuilt and run after this modification, it executes correctly again.
