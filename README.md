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
* tbb/2020.3 # not sure if that one is needed (should be not) <-----

## Information about the setup
This project contains the minimal required modules and code to recreate the issue encountered.

The code is compiled with the nvc++ compiler. Since CMake adds some flags (e.g. `-Wno-unused-local-typedefs`) to the compilation process that are valid for nvc++, an auxillary script (`nvc++p`) is used to strip them from the commands.

# TODO: describe setup for localrc

## Provoking the error
This requires that the Eigen and ActsCore library are compiled with gcc. Therefore, built in a separate workspace Eigen from sourc following these steps:
```
git clone https://gitlab.com/libeigen/eigen.git
cmake -S eigen -B eigen-build/ -DCMAKE_INSTALL_PREFIX=./eigen-3.3.9-gcc # NOTE: be sure that CXX compiler identification is GNU 11.2.0 and not PGI (i.e. nvc++)
cd eigen-build
make install
cd ..
rm -rf eigen-build 
```


hmmm... in this case it actually works already. Check if it depends on version




cmake -S . -B build -DCMAKE_CXX_COMPILER=$PWD/nvc++_p  && cmake --build build --parallel && ./build/Tutorial
// remove --parallel, or explain how to compile in two runs quicker


think if quickbuild should be included in git or not (rathr not)
