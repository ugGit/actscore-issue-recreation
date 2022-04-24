#include <boost/hana.hpp>

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
#include <chrono>

int main(int argc, char* argv[]){    
  //
  // Instantiate a vector of strings, because that causes a seg fault apparantly
  //
  std::vector<std::string> vec {"z", "y", "x"};
  std::cout << vec.at(0) << std::endl;
  
  //
  // Instantiate only a string, becaus this does not influence the program :')
  //
  std::string s = "mystring";
  std::cout << s << std::endl;

  //
  // Run an parallel algorithm using C++17
  //
  const int DSIZE = 2*32*1048576;

  // Initialize vectors
  std::vector<float> a(DSIZE);
  std::vector<float> b(DSIZE);
  std::vector<float> c(DSIZE);
  for (int i = 0; i < DSIZE; i++){
      a.at(i) = rand()/(float)RAND_MAX;
      b.at(i) = rand()/(float)RAND_MAX;
  }

  // start crono
  const auto t1 = std::chrono::high_resolution_clock::now();

  // execute 
  std::transform(std::execution::par_unseq, a.begin(), a.end(), b.begin(), c.begin(), [](float x, float y) -> float {return x+y;});
  
  // stop crono
  const auto t2 = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double, std::milli> ms = t2 - t1;
  
  std::cout << "Execution time [ms]: " << ms.count() << "\n";
  std::cout << "-----------\n";

  std::cout << a.at(0) << "\n";
  std::cout << b.at(0) << "\n";
  std::cout << c.at(0) << "\n";
  return 0;
}
