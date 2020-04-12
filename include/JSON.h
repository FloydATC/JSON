#ifndef JSON_HEADER
#define JSON_HEADER


#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>


class JSON_node;

enum class JSON_nodetype {
  JSON_nodetype_null,
  JSON_nodetype_boolean,
  JSON_nodetype_number,
  JSON_nodetype_string,
  JSON_nodetype_array,
  JSON_nodetype_object
};

typedef std::string JSON_stringtype;
typedef std::vector<JSON_node*> JSON_arraytype;
typedef std::vector<std::string> JSON_objectkeytype;
typedef std::unordered_map<std::string, JSON_node*> JSON_objectvaluetype;

union JSON_nodevalue {
  bool boolean_value;
  double number_value;
  JSON_stringtype* string_value;
  JSON_arraytype* array_value;
  JSON_objectvaluetype* objectvalues_value;
};


/*

  Each value in a JSON document is represented by a JSON_node
  ARRAY and OBJECT nodes may contain 0 or more child nodes

*/
class JSON_node {
  public:
    JSON_node(std::ifstream& filehandle);
    ~JSON_node();

    JSON_nodetype nodeType();

    void dump(std::ostream& os, uint8_t depth) const;

  protected:

  private:
    std::string find_object_key();
    std::string find_quoted_string();
    void parse_object();
    void parse_array();
    void parse_string();
    void parse_value();
    bool check_more_items();

    void dump_prefix(std::ostream& os, uint8_t depth) const;
    void dump_string(std::ostream& os, uint8_t depth) const;
    void dump_array(std::ostream& os, uint8_t depth) const;
    void dump_object(std::ostream& os, uint8_t depth) const;

    void consume(uint8_t c, std::string message);
    bool advance();
    uint8_t peek();
    void error(std::string message);
    bool is_eof();
    bool is_key();
    bool is_digit();
    bool is_alpha();
    bool is_whitespace();
    void skip_whitespace();


    std::ifstream& filehandle;
    uint8_t current;
    JSON_nodetype node_type;
    JSON_nodevalue node_value;
    JSON_objectkeytype* objectkeys_index;
};

/*

  JSON parser object - load/edit/dump/save JSON documents
  Can contain one JSON_node as "root"

*/

class JSON
{
  public:
    JSON();
    ~JSON();

    void load(std::string filename);
    void save(std::string filename) const;
    void dump(std::ostream& os) const;

  protected:

  private:
    JSON_node* root;

};

std::ostream& operator<< (std::ostream& os, const JSON& json);


#endif
