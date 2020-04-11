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




class JSON_node {
  public:
    JSON_node(std::ifstream& filehandle);
    ~JSON_node();

    std::ostream& dump(std::ostream& os, uint8_t depth);
    std::ostream& dump_prefix(std::ostream& os, uint8_t depth);

  protected:

  private:
    std::string return_object_key();
    void get_object();
    void get_array();
    void get_quoted_string();
    void get_value();
    void consume(uint8_t c, std::string message);
    bool advance();
    uint8_t peek();
    void error(std::string message);
    bool more_items();
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



class JSON
{
	public:
	  JSON();
	  ~JSON();

	  void load(std::string filename);
	  void save(std::string filename);

    JSON_node* root;

    friend std::ostream& operator<<(std::ostream& os, JSON& json)
    {
      if (json.root == nullptr) {
        os << "null" << std::endl;
      } else {
        json.root->dump(os, 0);
      }
      return os;
    }

	protected:

	private:

};





#endif
