#ifndef JSON_HEADER
#define JSON_HEADER


#include <memory>
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

typedef std::shared_ptr<JSON_node> JSON_nodeptr;
typedef std::string JSON_stringtype;
typedef std::vector<JSON_nodeptr> JSON_arraytype;
typedef std::unordered_map<std::string, JSON_nodeptr> JSON_objecttype;



/*

  Each value in a JSON document is represented by a JSON_node
  ARRAY and OBJECT nodes may contain 0 or more child nodes

*/
class JSON_node {
  public:
    JSON_node();
    JSON_node(double value);
    JSON_node(std::string value);
    JSON_node(std::istream* istream);
    ~JSON_node();

    JSON_nodetype nodeType();

    void dump(std::ostream& ostream) const;

    bool isNull();
    bool isBoolean();
    bool isNumber();
    bool isString();
    bool isArray();
    bool isObject();

    JSON_nodeptr getChildByIndex(size_t index);
    JSON_nodeptr getChildByKey(std::string key);
    void setChildByIndex(size_t index, JSON_node node);
    void setChildByKey(std::string key, JSON_node node);
    void addChildNode(JSON_node node);

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
    void dump(std::ostream& ostream, uint8_t depth) const;

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

    std::istream* istream;
    uint8_t current;
    JSON_nodetype node_type;

    bool boolean_value;
    double number_value;
    JSON_stringtype string_value;
    JSON_arraytype array_value;
    JSON_objecttype object_value;

};

std::ostream& operator<< (std::ostream& os, const JSON_node& json);


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
    void parse(std::istream& istream);
    void save(std::string filename) const;
    void dump(std::ostream& ostream) const;

    JSON_nodeptr getNode(std::string path);
    void setNode(std::string path, JSON_node node);
    void setBoolean(std::string path, bool value);
    void setNumber(std::string path, double value);
    void setString(std::string path, std::string string);

  protected:

  private:
    JSON_nodeptr root;

    std::vector<std::string> split_path_elements(std::string path);
    std::string join_path_elements(std::vector<std::string> elements);

};

std::ostream& operator<< (std::ostream& os, const JSON& json);
std::istream& operator>> (std::istream& is, JSON& json);


#endif
