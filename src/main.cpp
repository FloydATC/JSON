
#include <iostream>

#include "JSON.h"

int main(int argc, char* argv[])
{

  for (int i=1; i<argc; i++) {

    // Create JSON handler object
    JSON json = JSON();

    // Parse JSON document from a file
    json.load(argv[i]);

    // Pretty-print JSON to STDOUT
    std::cout << json << std::endl;

    // Get node by id
    std::cout << *json.getNode("quuz.3") << std::endl;

    // Write out JSON document to a file
    json.save("copy.json");

  }

  return 0;
}
