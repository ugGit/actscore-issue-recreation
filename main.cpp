/** TRACCC library, part of the ACTS project (R&D line)
 *
 * (c) 2021-2022 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#include <string>
#include <vector>
#include <iostream>
#include <execution>
#include <algorithm>

int main(int argc, char* argv[]){    
  //
  // Introduces the segmentation fault
  //
  std::vector<std::string> vec {"z", "y", "x"};
  std::cout << vec.at(0) << std::endl;

  //
  // Run an example to show the C++17 parallel execution policies work
  //
  const int DSIZE = 64*1048576;
  // Initialize vectors
  std::vector<float> a(DSIZE);
  std::vector<float> b(DSIZE);
  std::vector<float> c(DSIZE);
  for (int i = 0; i < DSIZE; i++){
      a.at(i) = rand()/(float)RAND_MAX;
      b.at(i) = rand()/(float)RAND_MAX;
  }
  // execute 
  std::transform(std::execution::seq, a.begin(), a.end(), b.begin(), c.begin(), [](float x, float y) -> float {return x+y;});
  // print out some results to verify calculation
  std::cout << "-----------\n";
  std::cout << a.at(0) << "\n";
  std::cout << b.at(0) << "\n";
  std::cout << c.at(0) << "\n";
  return 0;
}
