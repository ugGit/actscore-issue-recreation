# Issue recreation

During the quest of testing the parallel execution policies for C++17 Std Algorithms, a segmentation fault appears. Ironically, the error occurs during the clean up phase after having executed correctly the main program.

## Requirements
The issue has been debugged so far in an environment using the following modules:

* gcc/11.2.0
* nvhpc/22.3    
* boost/1.72.0  

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
Using the installed Eigen library on zeus.lbl.gov caused a compilation error in the style of:
```
"/opt/eigen/3.3.9/include/eigen3/Eigen/src/Core/arch/SSE/PacketMath.h", line 887: error: function "_mm_castpd_ps" has already been defined
  static inline __m128  _mm_castpd_ps   (__m128d x) { return reinterpret_cast<__m128&>(x);  }
                        ^
```

However, this can be worked around by installing the latest version of Eigen and link to it during the configuration step of CMake.
The following steps outline the installation process:
```
git clone https://gitlab.com/libeigen/eigen.git
cmake -S eigen -B eigen-build/ -DCMAKE_INSTALL_PREFIX=<whereeverYouLike> # NOTE: be sure that CXX compiler identification is GNU 11.2.0 and not PGI (i.e. nvc++)
cd eigen-build
make install
cd ..
rm -rf eigen-build 
```

And during the configuration of this project, the previously installed library shall be linked:
```
# cd to/this/directory/again
cmake -S . -B build -DCMAKE_CXX_COMPILER=$PWD/nvc++_p -DEigen3_DIR=<whereeverYouInstalledTheLib>/share/eigen3/cmake/
```

If the error persists, [somebody suggested a patch](https://gitlab.com/libeigen/eigen/-/issues/2470) just a few weeks ago. For convenience, the file `eigen.patch` is provided in this project and can be applied to your local clone of the Eigen project.

## Provoking the error
In a first run, the ActsCore and Eigen libs are compiled using gcc. This can be confirmed looking at the following array in the `nvc++p` script:
```
NVCPP_DIRS=(
    "TestActsCore.dir"
)
```

To compile the project using the auxillary script and pointing to the Eigen lib, execute the following commands (from within this directory):
```
cmake -S . -B build -DCMAKE_CXX_COMPILER=$PWD/nvc++_p
cmake --build build --parallel
```

To built executable can be run with:
```
./build/TestActsCore
```

And will result in a segmentation fault after having executd the code otherwise correctly:

```
z
  3  -1
2.5 1.5
Execution time [ms]: 87.5776
-----------
0.840188
0.394383
1.23457
Segmentation fault (core dumped)
```

It's maybe noteworthy, that the error changes when switching the parallel execution policy from `std::execution::par` to `std::execution::par_unseq`. After recompiling, we get the following error after correct programm execution:

```
*** Error in `./build/TestActsCore': free(): invalid pointer: 0x00007fc9a4008e80 ***
======= Backtrace: =========
/lib64/libc.so.6(+0x81329)[0x7fc9ff6a1329]
/lib64/libc.so.6(__cxa_finalize+0x9a)[0x7fc9ff65a05a]
/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/build/_deps/acts-build/lib64/libActsCore.so(+0x8e073)[0x7fca088d1073]
======= Memory map: ========
00400000-00478000 r-xp 00000000 08:51 408225910                          /bld6/users/nwachuch/nvcpp-tests/actscore-issue-recreation/build/TestActsCore
```

A first analysis with the debugger showed the following backtrace:

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

## Solving this error unelegantly

So far, there is no working solution, but a proof that solving what causes the segmentation fault here, will most likely solve the issue in traccc.

### Solution step 1: Build ActsCore with ncv++

This is done by adding the compilation directory to the array in the auxillary `nvc++p` script:
```
NVCPP_DIRS=(
    "TestActsCore.dir"
    "ActsCore.dir"
)
```

Rebuilding the project, and we should get the following compilation error:

```
"/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/build/_deps/acts-src/Core/include/Acts/Utilities/detail/MPL/type_collector.hpp", line 58: error: expression must have a constant value
        hana::filter(t_, [&](auto t) { return predicate(t); });
        ^
"/opt/boost/1.72.0/include/boost/hana/detail/ebo.hpp", line 62: note: attempt to access run-time storage
              : V(static_cast<T&&>(t))
                  ^
"/opt/boost/1.72.0/include/boost/hana/basic_tuple.hpp", line 69: note: called from:
              explicit constexpr basic_tuple_impl(Yn&& ...yn)
                                 ^
"/opt/boost/1.72.0/include/boost/hana/basic_tuple.hpp", line 96: note: called from:
          explicit constexpr basic_tuple(Yn&& ...yn)
                             ^
"/opt/boost/1.72.0/include/boost/hana/tuple.hpp", line 127: note: called from:
          constexpr tuple(Yn&& ...yn)
                    ^
"/opt/boost/1.72.0/include/boost/hana/tuple.hpp", line 316: note: called from:
          { return {static_cast<Xs&&>(xs)...}; }
            ^
"/opt/boost/1.72.0/include/boost/hana/fwd/core/make.hpp", line 61: note: called from:
              return make_impl<Tag>::apply(static_cast<X&&>(x)...);
                                          ^
"/opt/boost/1.72.0/include/boost/hana/filter.hpp", line 115: note: called from:
              return hana::make<S>(
                                  ^
"/opt/boost/1.72.0/include/boost/hana/filter.hpp", line 127: note: called from:
              return filter_impl::filter_helper<Indices>(
                                                        ^
"/opt/boost/1.72.0/include/boost/hana/filter.hpp", line 48: note: called from:
          return Filter::apply(static_cast<Xs&&>(xs),
                              ^
          detected during:
            instantiation of function "lambda [](auto, auto, auto)->auto [with <auto-1>=boost::hana::tuple<boost::hana::type<Acts::SurfaceCollector<Acts::MaterialSurface>>, boost::hana::type<Acts::VolumeCollector<Acts::MaterialVolume>>>, <auto-2>=boost::hana::type_detail::is_valid_fun<lambda [](auto)->boost::hana::type<<unnamed>::type::result_type> &&>, <auto-3>=boost::hana::template_t<Acts::detail::result_type_extractor::extractor_impl>]" at line 77
            instantiation of "const auto Acts::detail::type_collector_t [with helper=Acts::detail::result_type_extractor, items=<Acts::SurfaceCollector<Acts::MaterialSurface>, Acts::VolumeCollector<Acts::MaterialVolume>>]" at line 42 of "/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/build/_deps/acts-src/Core/include/Acts/Propagator/ActionList.hpp"
            instantiation of type "Acts::ActionList<actors_t...>::result_type<Acts::Propagator<Acts::StraightLineStepper, Acts::Navigator>::result_type_helper<Acts::SingleCurvilinearTrackParameters<Acts::SinglyCharged>, Acts::ActionList<Acts::SurfaceCollector<Acts::MaterialSurface>, Acts::VolumeCollector<Acts::MaterialVolume>>>::this_result_type> [with actors_t=<Acts::SurfaceCollector<Acts::MaterialSurface>, Acts::VolumeCollector<Acts::MaterialVolume>>]" at line 311 of "/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/build/_deps/acts-src/Core/include/Acts/Propagator/Propagator.hpp"
            instantiation of class "Acts::Propagator<stepper_t, navigator_t>::result_type_helper<parameters_t, action_list_t> [with stepper_t=Acts::StraightLineStepper, navigator_t=Acts::Navigator, parameters_t=Acts::SingleCurvilinearTrackParameters<Acts::SinglyCharged>, action_list_t=Acts::ActionList<Acts::SurfaceCollector<Acts::MaterialSurface>, Acts::VolumeCollector<Acts::MaterialVolume>>]" at line 216 of "/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/build/_deps/acts-src/Core/src/Material/SurfaceMaterialMapper.cpp"

"/home/nwachuch/bld6/nvcpp-tests/actscore-issue-recreation/build/_deps/acts-src/Core/src/Material/SurfaceMaterialMapper.cpp", line 217: error: type name is not allowed
    auto mcResult = result.get<MaterialSurfaceCollector::result_type>();
                               ^
```

### Solution step 2: Remove code causing the error
Sure, this is not a solution to work with, but it proves a point: we're digging at the right place. 

Based on the information from the follow up error, we know the code causing trouble is in the `SurfaceMaterialMapper.cpp` file at line 217. However, the preceeding error is caused by the previous line 216. 

Since I couldn't find a real solution, I simply removed the code in `.../build/_deps/acts-src/Core/src/Material/SurfaceMaterialMapper.cpp" from line 216 on. I suggest, you do the same, just make sure to keep the last closing bracket for the current code block.

Now you could rebuild, and observe that the same happens in the `VolumeMaterialMapper.cpp` file at line 298. Therefore, the same brutal procedure is applied and the lines 297 till the end are removed (except for the last closing bracket).

With these changes applied, the project should compile and **RUN without segmentation fault!**

## A small bonus fun fact
If the Eigen is not used in `main.cpp`, i.e. no matrix is created and outputted, the segmentation fault is back. I could now guessing why, but honestly, do not see why... yet.
