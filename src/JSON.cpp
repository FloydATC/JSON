#include "JSON.h"



JSON::JSON()
{
	this->root = nullptr;
}


JSON::~JSON()
{
	if (this->root != nullptr) delete this->root;
}


void JSON::load(std::string filename)
{
	if (this->root != nullptr) delete this->root;

  std::ifstream filehandle(filename);
  if (filehandle.is_open()) {
    this->root = new JSON_node(filehandle);
    filehandle.close();
  }
}


void JSON::save(std::string filename)
{
	if (this->root == nullptr) return;
	
}



JSON_node::JSON_node(std::ifstream& filehandle) : filehandle(filehandle)
{
  this->skip_whitespace();
  // Determine node type
  switch(this->peek()) {
    case '[': this->get_array(); break;
    case '{': this->get_object(); break;
    case '"': this->get_quoted_string(); break;
    default: this->get_value(); break;
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
      delete (JSON_stringtype*)this->node_value.string_value; 
      break;
    case JSON_nodetype::JSON_nodetype_array: 
      for (auto& value : *this->node_value.array_value) delete value;
      delete (JSON_arraytype*)this->node_value.array_value; 
      break;
    case JSON_nodetype::JSON_nodetype_object: 
      for (auto& pair : *this->node_value.objectvalues_value) delete pair.second;
      delete (JSON_objectkeytype*)this->objectkeys_index; 
      delete (JSON_objectvaluetype*)this->node_value.objectvalues_value; 
      break;
  }
}


bool JSON_node::more_items()
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

void JSON_node::get_array()
{
  // Initialize self as array node
  this->node_type = JSON_nodetype::JSON_nodetype_array;
  this->node_value.array_value = new JSON_arraytype();

  this->advance(); // Consume prefix bracket
  this->skip_whitespace();
  while(this->current != ']') {

    // Consume next array item
    JSON_node* child = new JSON_node(this->filehandle);

    // Add item to self
    this->node_value.array_value->push_back(child);

    if (!this->more_items()) break;
  }
  this->consume(']', "Expected ] after last array value");
}


std::string JSON_node::return_object_key()
{
  std::string key;
  while(true) {
    this->advance();
    key += this->current;
    if (!this->is_key()) break;
  }
  return key;
}


void JSON_node::get_object()
{
  // Initialize self as object node
  this->node_type = JSON_nodetype::JSON_nodetype_object;
  this->objectkeys_index = new JSON_objectkeytype();
  this->node_value.objectvalues_value = new JSON_objectvaluetype();

  this->advance(); // Consume prefix curly
  this->skip_whitespace();
  while(this->peek() != '}') {

    // Consume next item key
    std::string key = return_object_key();

    this->consume(':', "Expected : separator after object key");

    // Consume next item value
    JSON_node* value = new JSON_node(filehandle);

    // Add key/value pair to self
    this->objectkeys_index->push_back( key );
    this->node_value.objectvalues_value->insert({ key, value });

    if (!this->more_items()) break;
  }
  this->consume('}', "Expected } after last object value");
}


void JSON_node::get_quoted_string()
{
  // Get quoted string
  // Assign to self

  // Initialize self as string node
  this->node_type = JSON_nodetype::JSON_nodetype_string;

  std::string string;
  this->advance(); // Consume prefix quote
  while(true) {
    this->advance();
    string += this->current;
    if (this->peek() == '"') break;
  }

  this->advance(); // Consume suffix quote
  this->node_value.string_value = new std::string(string);
}


void JSON_node::get_value()
{
  // Get a numeric, boolean or null value
  // Assign to self
  std::string string;
  if (this->is_digit() || this->current == '.') {
    // Initialize self as number node
    this->node_type = JSON_nodetype::JSON_nodetype_number;

    // Consume number string
    while (true) {
      this->advance();
      string += this->current;
      if (!this->is_digit() && this->peek() != '.') break;
    }

    // Convert to double
    std::size_t offset = 0;
    this->node_value.number_value = std::stod(string, &offset);
    return;
  } else {
    // Must be either "null", "true" or "false"
    if (this->is_alpha()) {
      string += this->current;
      this->advance();
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
  this->filehandle.get(c);
  this->current = c;
  return true;
}


uint8_t JSON_node::peek()
{
  return this->filehandle.peek();
}


void JSON_node::error(std::string message)
{
  throw std::runtime_error(message);
}


bool JSON_node::is_eof()
{
  return this->filehandle.eof();
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


std::ostream& JSON_node::dump(std::ostream& os, uint8_t depth)
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
      os << '"' << this->node_value.string_value << '"';
      break;
    case JSON_nodetype::JSON_nodetype_array: 
      if (this->node_value.array_value->size() > 0) {
        os << "[" << std::endl;
        for (auto& value : *this->node_value.array_value) value->dump(os, depth+1);
        os << "]";
      } else {
        os << "[]";
      }
      break;
    case JSON_nodetype::JSON_nodetype_object: 
      if (this->objectkeys_index->size() > 0) {
        os << "{" << std::endl;
        for (auto& key : *this->objectkeys_index) {
          this->node_value.objectvalues_value->at(key)->dump(os, depth+1);
        }
        os << "}";
      } else {
        os << "{}";
      }
      break;
  }
  os << std::endl;
  return os;
}

