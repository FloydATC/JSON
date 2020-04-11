
#include <iostream>

#include "JSON.h"

int main(int argc, char* argv[])
{

  for (int i=1; i<argc; i++) {

    // Create JSON handler object
    JSON json = JSON();

    // Parse JSON document
    json.load(argv[i]);

    // Pretty-print JSON to STDOUT
    std::cout << json << std::endl;

  }

  return 0;
}
