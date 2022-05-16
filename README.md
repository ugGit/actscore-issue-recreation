### Update 1: Change of hardware (May 13)
The code has initially been tested on a Nvidia GeForce RTX 2080. 
A few days ago, the a A6000 has been added. 
Rerunning the issue on both GPUs yielded slightly different results, i.e. the code runs successfully for `std::execution::par_unseq` AND `std::execution::par`.
The software versions used in the test remained unchanged, but maybe a driver got updated along the way that caused this favorable outcome.

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

## Running the code successfully
The current state should allow compiling, linking, and running the program successfully. Therefore, pass the auxiliary script `nvc++_p` as compiler during the configuration of CMake as follows:
```
cmake -S . -B build -DCMAKE_CXX_COMPILER=$PWD/nvc++_p
```

Then build and execute the program:
```
cmake --build build/ --parallel
./build/TestActsCore
```

Which should yield the following output:
```
z
  3  -1
2.5 1.5
-----------
0.840188
0.394383
1.23457
```

## Provoking the error
However, the code in the example does not really reflect reality. 
We would like to experiment with different execution policies. 
Also, we do not have a matrix calculation in the file, so we'd like to remove it. 
Both changes result in a segmentation fault of the program.

As stated, this is all related to linking a dependency: [ActsCore](https://github.com/acts-project/acts). If the library is not linked in the CMake file, no segmentation fault occurs. This library is required in our real-world project. A first approach to solve this issue was to compile this dependency using `nvc++` instead of `gcc`. This, however, fails during the compilation of meta functions. More information and reproduction steps can be found here: https://github.com/ugGit/acts.

### First cause: changing the execution policy
Trying to change the execution policy from `par_unseq` to `par` in `main.cpp` results in a segmentation fault of the program. I.e. replacing:
```
std::transform(std::execution::par_unseq, a.begin(), a.end(), b.begin(), c.begin(), [](float x, float y) -> float {return x+y;});
```

with:

```
std::transform(std::execution::par, a.begin(), a.end(), b.begin(), c.begin(), [](float x, float y) -> float {return x+y;});
```

Rebuilding the project and executing the program now yields a segmentation fault after having executed the code otherwise correctly:

```
z
  3  -1
2.5 1.5
-----------
0.840188
0.394383
1.23457
Segmentation fault (core dumped)
```

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


### Second cause: removing the matrix calculation
Before continuing, the work environment should be reset to match the state on the current git main branch again (e.g. by executing `git checkout main.cpp`). 
Then, apply the provided patch which removes the matrix calculation from `main.cpp` using:

```
git apply provoke_error_remove_matrix.patch
```

Rebuilding the project and executing the program now yields a `free(): invalid pointer` error after having executed the code otherwise correctly:

```
z
-----------
0.840188
0.394383
1.23457

*** Error in `./build/TestActsCore': free(): invalid pointer: 0x00007fc9a4008e80 ***
======= Backtrace: =========
/lib64/libc.so.6(+0x81329)[0x7fc9ff6a1329]
/lib64/libc.so.6(__cxa_finalize+0x9a)[0x7fc9ff65a05a]
/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/build/_deps/acts-build/lib64/libActsCore.so(+0x8e073)[0x7fca088d1073]
======= Memory map: ========
00400000-00478000 r-xp 00000000 08:51 408225910                          /bld6/users/nwachuch/nvcpp-tests/actscore-issue-recreation/build/TestActsCore
...
```

## Conclusions so far

Based on the error log and the debugger output above, we assume the error is caused by the destruction of a `string` within a `vector`. This can also be confirmed, by commenting the creation of the vector of strings (`std::vector<std::string> vec {"z", "y", "x"};`). If the project is rebuilt and run after this modification, it executes correctly again. This works regardless of the code instantiating an Eigen matrix and the execution policy selected.

One interesting observation made during the research of this issue, is the fact that compiling for `DEBUG` results also in a correct running program without the matrix code in `main.cpp` when using `par_unseq` as execution policy. I.e. using `cmake -S . -B build -DCMAKE_CXX_COMPILER=$PWD/nvc++_p -DCMAKE_BUILD_TYPE=Debug` for configuration.

Another interesting observation is that the project compiles and runs successfully for `multicore` targets.

An additional possible impact factor might be that the code is only executed in a kernel when using `par_unseq`. Using another execution policy results in the code being executed on the CPU. 
The computation capability of the GPU used is 75, and thus, we took notice of the following warning but did not investigate it any further:
```
"/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/main.cpp", line 49: warning: Calls to function "std::transform(_EP &&, _FIt1, _FIt1, _FIt2, _FIt3, _BF) [with _EP=const std::execution::parallel_policy &, _FIt1=__gnu_cxx::__normal_iterator<float *, std::vector<float, std::allocator<float>>>, _FIt2=__gnu_cxx::__normal_iterator<float *, std::vector<float, std::allocator<float>>>, _FIt3=__gnu_cxx::__normal_iterator<float *, std::vector<float, std::allocator<float>>>, _BF=struct lambda []]" with execution policy std::execution::par will run sequentially when compiled for a compute capability less than cc70; only std::execution::par_unseq can be run in parallel on such GPUs
    std::transform(std::execution::par, a.begin(), a.end(), b.begin(), c.begin(), [](float x, float y) -> float {
```

As stated at the very beginning, the issue only starts appearing, when `ActsCore` is linked. Therefore, not linking it to the executable in the buildsystem solves the issue as well.
