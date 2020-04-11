
#include <iostream>

#include "JSON.h"

int main(int argc, char* argv[])
{
  std::cout << "main() begin" << std::endl;

  for (int i=1; i<argc; i++) {
    std::cout << "main() loading \"" << argv[i] << "\"" << std::endl;
    JSON json = JSON();
    json.load(argv[i]);

    std::cout << "main() printing \"" << argv[i] << "\"" << std::endl;
    std::cout << json;

  }

  std::cout << "main() end" << std::endl;
  return 0;
}
