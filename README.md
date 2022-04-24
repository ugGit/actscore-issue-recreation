# Issue recreation

During the quest of testing the parallel execution policies for C++17 Std Algorithms, a segmentation fault appears. Ironically, the error occurs during the clean up phase after having executed correctly the main program.

A first analysis with the debugger shows the following backtrace:

```
Thread 1 "traccc_stdpar_p" received signal SIGSEGV, Segmentation fault.
0x000000000042ab4c in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > () at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/basic_string.h:187
187          { return _M_dataplus._M_p; }
(gdb) backtrace
#0  0x000000000042ab4c in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > () at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/basic_string.h:187
#1  0x000000000042acd5 in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > () at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/basic_string.h:222
#2  0x000000000042ad15 in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > () at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/basic_string.h:231
#3  0x000000000042b6d5 in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > () at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/basic_string.h:658
#4  0x000000000043a515 in std::_Destroy (__pointer=0x7fff92008e80)
    at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/stl_construct.h:98
#5  0x0000000000424c32 in __destroy (__first=0x7fff92008e80, __last=0x7fff92008fa0)
    at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/stl_construct.h:108
#6  0x000000000043a4ed in std::_Destroy (__first=0x7fff92008e80, __last=0x7fff92008fa0)
    at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/stl_construct.h:136
#7  0x000000000043a6f1 in std::_Destroy (__first=0x7fff92008e80, __last=0x7fff92008fa0, 
    _T51_119482=0x7ffff7485c70 <Acts::binningValueNames>)
    at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/stl_construct.h:206
#8  0x000000000041b081 in std::vector<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > ()
    at /bld4/opt/gcc/9.3.0/include/c++/9.3.0/bits/stl_vector.h:677
#9  0x00007fffebd4d05a in __cxa_finalize () from /usr/lib64/libc.so.6
#10 0x00007ffff58e8ca3 in __do_global_dtors_aux ()
   from /home/nwachuch/bld6/traccc/build/_deps/acts-build/lib64/libActsCore.so
#11 0x00007fffffffd350 in ?? ()
#12 0x00007ffff7deb08a in _dl_fini () from /lib64/ld-linux-x86-64.so.2
```

## Requirements
The issue has been debugged so far in an environment using the following modules:

* gcc/11.2.0
* nvhpc/22.3    
* boost/1.72.0  
* root/6.24.06-gcc112-c17

## Information about the setup
This project contains the minimal required modules and code to recreate the issue encountered.

Some code is compiled with the nvc++ compiler. Since CMake adds some flags (e.g. `-Wno-unused-local-typedefs`) to the compilation process that are valid for nvc++, an auxillary script (`nvc++p`) is used to strip them from the commands.

To start the reproduction, only one part will be compiled with the nvc++ compiler, whereas gcc is used for the rest. The dynamic selection of compiler within CMake is not possible. To work around this issue, a switch is built in to the auxillary script which defines the compiler per compilation directory.

## Configuration
### Localrc for nvc++
To run the nvc++ compiler with the desired gcc version, a localrc file must be generated. This can be done by executing a script provided by nvhpc:

```
makelocalrc -gcc PATH_TO_GCC -gpp PATH_TO_G++ -x -d PATH_TO_LOCALRC_DIR
```

Then, the auxillary script (`nvc++p`) must be updated such that the variable `LOCALRC` points to PATH_TO_LOCALRC_DIR defined in aboves command. 

### Eigen
For a proper step through the solution steps, we build from the very beginning the Eigen library separately  following these steps:
```
git clone https://gitlab.com/libeigen/eigen.git
cmake -S eigen -B eigen-build/ -DCMAKE_INSTALL_PREFIX=<whereeverYouLike>/eigen-gcc # NOTE: be sure that CXX compiler identification is GNU 11.2.0 and not PGI (i.e. nvc++)
cd eigen-build
make install
cd ..
rm -rf eigen-build 
```

## Provoking the error
In a first run, the ActsCore and Eigen libs are compiled using gcc. This can be confirmed looking at the following array in the `nvc++p` script:
```
NVCPP_DIRS=(
    "TestActsCore.dir"
)
```

To compile the project using the auxillary script and pointing to the Eigen lib, execute the following commands (from within this directory):
```
cmake -S . -B build -DCMAKE_CXX_COMPILER=$PWD/nvc++_p -DEigen3_DIR=<whereeverYouInstalledTheLib/eigen-gcc/cmake/
cmake --build build --parallel
```

To built executable can be run with:
```
./build/TestActsCore
```

And will result in a segmentation fault after having executd the code otherwise correctly:
```
z
mystring
Execution time [ms]: 87.5776
-----------
0.840188
0.394383
1.23457
Segmentation fault (core dumped)
```

It's also noteworthy, that the error changes when switching the parallel execution policy from `std::execution::par` to `std::execution::par_unseq`. After recompiling, we get the following error after correct programm execution:

```
*** Error in `./build/TestActsCore': free(): invalid pointer: 0x00007fc9a4008e80 ***
======= Backtrace: =========
/lib64/libc.so.6(+0x81329)[0x7fc9ff6a1329]
/lib64/libc.so.6(__cxa_finalize+0x9a)[0x7fc9ff65a05a]
/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/build/_deps/acts-build/lib64/libActsCore.so(+0x8e073)[0x7fca088d1073]
======= Memory map: ========
00400000-00478000 r-xp 00000000 08:51 408225910                          /bld6/users/nwachuch/nvcpp-tests/actscore-issue-recreation/build/TestActsCore
```

## Solving this error unelegantly

So far, there is no working solution, but a proof that solving what causes the segmentation fault here, will most likely solve the issue in traccc.

### Solution step 1: Build ActsCore with ncv++
