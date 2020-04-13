#include "JSON.h"

#include <sstream>
#include <algorithm>

std::ostream& operator<< (std::ostream& ostream, const JSON& json)
{
  json.dump(ostream);
  return ostream;
}


std::istream& operator>> (std::istream& istream, JSON& json)
{
  json.parse(istream);
  return istream;
}


JSON::JSON()
{
  std::stringstream sstream;
  sstream << "null";
  this->root = new JSON_node(&sstream);
}


JSON::~JSON()
{
  delete this->root;
}


void JSON::load(std::string filename)
{
  std::ifstream filehandle(filename);
  if (filehandle.is_open()) {
    filehandle >> *this;
    filehandle.close();
  }
}


void JSON::parse(std::istream& istream)
{
  delete this->root;
  this->root = new JSON_node(&istream);
}


void JSON::save(std::string filename) const
{
  if (this->root == nullptr) return;

  std::ofstream filehandle(filename);
  if (filehandle.is_open()) {
    filehandle << *this << std::endl;
    filehandle.close();
  }
}


void JSON::dump(std::ostream& ostream) const
{
  ostream << *this->root;
}


std::vector<std::string> JSON::split_path_elements(std::string path)
{
  std::vector<std::string> elements; 
  std::stringstream sstream(path);
  std::string id;
  while (getline(sstream, id, '.')) elements.push_back(id);
  return elements;
}


std::string JSON::join_path_elements(std::vector<std::string> elements)
{
  return "";
}


JSON_node* JSON::getNode(std::string path)
{
  JSON_node* parent_node = this->root;

  for (auto& id : this->split_path_elements(path)) {
    // Is the parent node an array?
    if (parent_node->isArray()) {
      parent_node = parent_node->getChildByIndex(std::stoul(id));
      if (parent_node != nullptr) continue;
    }
    // Is the parent node an object?
    if (parent_node->isObject()) {
      parent_node = parent_node->getChildByKey(id);
      if (parent_node != nullptr) continue;
    }
    // None of the above? Then it can't have an id
    return nullptr;
  }
  return parent_node;
}


void JSON::setNode(std::string path, JSON_node* node)
{
  if (path == "") {
    delete this->root;
    this->root = node;
  } else {
    std::vector<std::string> elements = this->split_path_elements(path);
    std::string id = elements.back();
    elements.pop_back();
    std::string parent_path = this->join_path_elements(elements);
    JSON_node* parent_node = this->getNode(parent_path);
    if (parent_node == nullptr) {
      delete node;
      throw std::runtime_error("Not found: '" + path + "'");
    }
    // Is the parent node an array?
    if (parent_node->isArray()) {
      parent_node->setChildByIndex(std::stoul(id), node);
      return;
    }
    // Is the parent node an object?
    if (parent_node->isObject()) {
      parent_node->setChildByKey(id, node);
      return;
    }
    // None of the above? Then it can't have an id
    if (parent_node == nullptr) {
      delete node;
      throw std::runtime_error("Not a container node: '" + parent_path + "'");
    }
  }
}


void JSON::setBoolean(std::string path, bool value) 
{
  std::stringstream sstream;
  sstream << (value ? "true" : "false");
  this->setNode(path, new JSON_node(&sstream) );
}


void JSON::setNumber(std::string path, double value) 
{
  this->setNode(path, new JSON_node(value) );
}


void JSON::setString(std::string path, std::string value) 
{
  this->setNode(path, new JSON_node(value) );
}




/*

  The real work is carried out by the JSON_node class

*/


std::ostream& operator<< (std::ostream& ostream, const JSON_node& json_node)
{
  json_node.dump(ostream);
  return ostream;
}


JSON_node::JSON_node()
{
  // Set default node type = null
  this->node_type = JSON_nodetype::JSON_nodetype_null;
  this->istream = nullptr;
}


JSON_node::JSON_node(double value)
{
  this->node_type = JSON_nodetype::JSON_nodetype_number;
  this->node_value.number_value = value;
  this->istream = nullptr;
}


JSON_node::JSON_node(std::string value)
{
  this->node_type = JSON_nodetype::JSON_nodetype_string;
  this->node_value.string_value = new std::string(value);
  this->istream = nullptr;
}


JSON_node::JSON_node(std::istream* istream)
{
  // Set default node type = null
  this->node_type = JSON_nodetype::JSON_nodetype_null;

  this->istream = istream;
  this->skip_whitespace();

  // Determine node type
  switch(this->peek()) {
    case '[': this->parse_array(); break;
    case '{': this->parse_object(); break;
    case '"': this->parse_string(); break;
    default: this->parse_value(); break;
  }

}


JSON_node::~JSON_node()
{
  switch(this->node_type) {
    case JSON_nodetype::JSON_nodetype_null: 
      break;
    case JSON_nodetype::JSON_nodetype_boolean: 
      break;
    case JSON_nodetype::JSON_nodetype_number: 
      break;
    case JSON_nodetype::JSON_nodetype_string: 
      delete this->node_value.string_value; 
      break;
    case JSON_nodetype::JSON_nodetype_array: 
      for (auto& value : *this->node_value.array_value) delete value;
      delete this->node_value.array_value; 
      break;
    case JSON_nodetype::JSON_nodetype_object: 
      for (auto& pair : *this->node_value.object_value) delete pair.second;
      delete this->node_value.object_value; 
      break;
  }
}


JSON_nodetype JSON_node::nodeType()
{
  return this->node_type;
}


bool JSON_node::check_more_items()
{
  this->skip_whitespace();
  if (this->peek() == ',') {
    this->advance(); // Consume separating comma
    this->skip_whitespace();
    return true;
  } else {
    return false;
  }
}


void JSON_node::parse_array()
{
  // Initialize self as array node
  this->node_type = JSON_nodetype::JSON_nodetype_array;
  this->node_value.array_value = new JSON_arraytype();

  this->advance(); // Consume prefix bracket
  this->skip_whitespace();
  while(this->peek() != ']') {

    // Consume next array item
    JSON_node* child = new JSON_node(this->istream);

    // Add item to self
    this->node_value.array_value->push_back(child);

    // Look for comma
    if (!this->check_more_items()) break;
  }
  this->consume(']', "Expected ] after last array value");
}


std::string JSON_node::find_object_key()
{
  std::string key;
  // Strictly speaking, unquoted keys are disallowed in JSON 
  // but it's a very common error so we allow it when parsing

  if (this->peek() == '"') {
    key = this->find_quoted_string();
  } else {
    while(true) {
      this->advance();
      key += this->current;
      if (!this->is_key()) break;
    }
  }
  return key;
}


std::string JSON_node::find_quoted_string()
{
  std::string string;

  this->advance(); // Consume prefix quote

  while(true) {
    if (this->peek() == '"' || this->is_eof()) break;
    if (this->peek() == '\\') {
      this->advance(); // Consume backslash
      // Escape sequence
      switch (this->peek()) {
        // Only advance if the escape sequence is valid
        case 'b': this->advance(); string += '\b'; continue;
        case 'f': this->advance(); string += '\f'; continue;
        case 'n': this->advance(); string += '\n'; continue;
        case 'r': this->advance(); string += '\r'; continue;
        case 't': this->advance(); string += '\t'; continue;
        case '"': this->advance(); string += '"'; continue;
        case '\\': this->advance(); string += '\\'; continue;
        // Error message will contain the bad character
        default: this->error("Bad escape sequence");
      }
    } 
    this->advance(); // Ordinary character
    string += this->current;
  }

  this->advance(); // Consume suffix quote

  return string;
}


void JSON_node::parse_object()
{
  // Initialize self as object node
  this->node_type = JSON_nodetype::JSON_nodetype_object;
  this->node_value.object_value = new JSON_objecttype();

  this->advance(); // Consume prefix curly
  this->skip_whitespace();
  while(this->peek() != '}') {

    // Consume next item key
    std::string key = find_object_key();

    // Look for colon
    this->consume(':', "Expected : separator after object key '" + key + "'");

    // Consume next item value
    JSON_node* value = new JSON_node(this->istream);

    // Add key/value pair to self
    this->node_value.object_value->insert({ key, value });

    // Look for comma
    if (!this->check_more_items()) break;
  }
  this->consume('}', "Expected } after last object value");
}


void JSON_node::parse_string()
{
  // Get quoted string
  // Assign to self

  // Initialize self as string node
  this->node_type = JSON_nodetype::JSON_nodetype_string;

  std::string string = this->find_quoted_string();
  this->node_value.string_value = new std::string(string);
}


void JSON_node::parse_value()
{
  // Get a numeric, boolean or null value
  // Assign to self
  std::string string;
  if (this->is_digit() || this->peek() == '-') {
    // Initialize self as number node
    this->node_type = JSON_nodetype::JSON_nodetype_number;

    // Consume number string
    while (true) {
      this->advance();
      string += this->current;
      if (!this->is_digit() && this->peek() != '.') break;
    }

    // Convert to double
    size_t offset = 0;
    this->node_value.number_value = std::stod(string, &offset);
    return;
  } else {
    // Must be either "null", "true" or "false"
    while (true) {
      this->advance();
      string += this->current;
      if (!this->is_alpha()) break;
    }
    if (string == "null") {
      // Initialize self as null node
      this->node_type = JSON_nodetype::JSON_nodetype_null;
      return;
    }
    if (string == "true") {
      // Initialize self as boolean node
      this->node_type = JSON_nodetype::JSON_nodetype_boolean;
      this->node_value.boolean_value = true;
      return;
    }
    if (string == "false") {
      // Initialize self as boolean node
      this->node_type = JSON_nodetype::JSON_nodetype_boolean;
      this->node_value.boolean_value = false;
      return;
    }
    this->error("Invalid plaintext value");
  }
}




/*

  Low level parsing functions

*/

void JSON_node::error(std::string message)
{
  std::string errormsg = message + " near byte " + std::to_string(this->istream->tellg()) + ": '" + (const char)this->peek() + "'";
  throw std::runtime_error(errormsg);
}


void JSON_node::consume(uint8_t c, std::string message)
{
  this->skip_whitespace();
  if (this->peek() != c) this->error(message);
  this->advance(); // Consume character
}


bool JSON_node::advance()
{
  if (this->is_eof()) return false;
  char c;
  this->istream->get(c);
  this->current = c;
  return true;
}


uint8_t JSON_node::peek()
{
  if (this->is_eof()) return 0;
  return this->istream->peek();
}


bool JSON_node::is_eof()
{
  return this->istream->eof();
}


bool JSON_node::is_key()
{
  char c = this->peek();
  if (c >= '0' && c <= '9') return true;
  if (c >= 'a' && c <= 'z') return true;
  if (c >= 'A' && c <= 'Z') return true;
  if (c == '_') return true;
  return false;
}


bool JSON_node::is_digit()
{
  char c = this->peek();
  if (c >= '0' && c <= '9') return true;
  return false;
}


bool JSON_node::is_alpha()
{
  char c = this->peek();
  if (c >= 'a' && c <= 'z') return true;
  if (c >= 'A' && c <= 'Z') return true;
  return false;
}


bool JSON_node::is_whitespace()
{
  char c = this->peek();
  if (c == ' ') return true;
  if (c == '\t') return true;
  if (c == '\n') return true;
  return false;
}


void JSON_node::skip_whitespace()
{
  while (this->is_whitespace() && !this->is_eof()) this->advance();
}




/*

  Dump functions

*/


void JSON_node::dump_prefix(std::ostream& os, uint8_t depth) const
{
  for (uint8_t i=0; i<depth; i++) os << "  ";
}


void JSON_node::dump_string(std::ostream& os, uint8_t depth) const
{
  os << '"';
  for (auto &c : *this->node_value.string_value) {
    switch (c) {
      case '\b': os << "\\b"; break;
      case '\f': os << "\\f"; break;
      case '\n': os << "\\n"; break;
      case '\r': os << "\\r"; break;
      case '\t': os << "\\t"; break;
      case '"': os << "\\" << '"'; break;
      case '\\': os << "\\\\"; break;
      default: os << c;
    }
  }
  os << '"';
}


void JSON_node::dump_array(std::ostream& os, uint8_t depth) const
{
  size_t count = this->node_value.array_value->size();
  size_t index = 0;
  if (count > 0) {
    os << "[" << std::endl;

    for (auto& value : *this->node_value.array_value) {
      this->dump_prefix(os, depth+1);
      value->dump(os, depth+1);
      index++;
      os << (index < count ? "," : "") << std::endl;
    }

    this->dump_prefix(os, depth);
    os << "]";
  } else {
    os << "[]";
  }
}


void JSON_node::dump_object(std::ostream& os, uint8_t depth) const
{
  size_t count = this->node_value.object_value->size();
  size_t index = 0;
  if (count > 0) {
    os << "{" << std::endl;

    // Get keys in sort order
    std::vector<std::string> keys;
    for (auto& pair : *this->node_value.object_value) keys.push_back(pair.first);
    std::sort(keys.begin(), keys.end());

    // Dump key/value pairs
    for (auto& key : keys) {
      this->dump_prefix(os, depth+1);
      os << "\"" << key << "\": ";
      JSON_node* child = this->node_value.object_value->at(key);
      child->dump(os, depth+1);
      index++;
      os << (index < count ? "," : "") << std::endl;
    }

    this->dump_prefix(os, depth);
    os << "}";
  } else {
    os << "{}";
  }
}


void JSON_node::dump(std::ostream& os) const
{
  this->dump(os, 0);
}


void JSON_node::dump(std::ostream& os, uint8_t depth) const
{
  switch(this->node_type) {
    case JSON_nodetype::JSON_nodetype_null: 
      os << "null";
      break;
    case JSON_nodetype::JSON_nodetype_boolean: 
      os << (this->node_value.boolean_value ? "true" : "false");
      break;
    case JSON_nodetype::JSON_nodetype_number: 
      os << std::to_string(this->node_value.number_value);
      break;
    case JSON_nodetype::JSON_nodetype_string: 
      this->dump_string(os, depth);
      break;
    case JSON_nodetype::JSON_nodetype_array: {
      this->dump_array(os, depth);
      break;
    }
    case JSON_nodetype::JSON_nodetype_object: {
      this->dump_object(os, depth);
      break;
    }
  }
}



bool JSON_node::isNull()
{
  return this->node_type == JSON_nodetype::JSON_nodetype_null;
}


bool JSON_node::isBoolean()
{
  return this->node_type == JSON_nodetype::JSON_nodetype_null;
}


bool JSON_node::isNumber()
{
  return this->node_type == JSON_nodetype::JSON_nodetype_number;
}


bool JSON_node::isString()
{
  return this->node_type == JSON_nodetype::JSON_nodetype_string;
}


bool JSON_node::isArray()
{
  return this->node_type == JSON_nodetype::JSON_nodetype_array;
}


bool JSON_node::isObject()
{
  return this->node_type == JSON_nodetype::JSON_nodetype_object;
}


JSON_node* JSON_node::getChildByIndex(size_t index)
{
  if (this->node_type != JSON_nodetype::JSON_nodetype_array) return nullptr;
  if (index >= this->node_value.array_value->size()) return nullptr;
  return this->node_value.array_value->at(index);
}


JSON_node* JSON_node::getChildByKey(std::string key)
{
  if (this->node_type != JSON_nodetype::JSON_nodetype_object) return nullptr;
  if (this->node_value.object_value->count(key) == 0) return nullptr;
  return this->node_value.object_value->at(key);
}


void JSON_node::setChildByIndex(size_t index, JSON_node* node)
{
  if (this->node_type != JSON_nodetype::JSON_nodetype_array) {
    delete node;
    throw std::runtime_error("Not an array node");
  }
  if (index > this->node_value.array_value->size() -1) {
    delete node;
    throw std::runtime_error("Index out of range");
  }
  // Destroy existing node before replacing
  delete this->node_value.array_value->at(index);
  this->node_value.array_value->at(index) = node;
}


void JSON_node::setChildByKey(std::string key, JSON_node* node)
{
  if (this->node_type != JSON_nodetype::JSON_nodetype_object) {
    delete node;
    throw std::runtime_error("Not an array node");
  }
  if (this->node_value.object_value->count(key) > 0) {
    // Destroy existing node before replacing
    delete this->node_value.object_value->at(key);
  }
  this->node_value.object_value->insert( {key, node} );
}


void JSON_node::addChildNode(JSON_node* node)
{
  if (this->node_type != JSON_nodetype::JSON_nodetype_array) {
    delete node;
    throw std::runtime_error("Not an array node");
  }
  this->node_value.array_value->push_back(node);
}

