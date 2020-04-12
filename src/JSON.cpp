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

    // Special case: If the root node is of type NULL,
    // the JSON document is empty and the JSON_node object is pointless
    if (this->root->nodeType() == JSON_nodetype::JSON_nodetype_null) {
      delete this->root;
      this->root = nullptr;
    }

  }
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


void JSON::dump(std::ostream& os) const
{
  if (this->root == nullptr) {
    os << "null" << std::endl;
  } else {
    this->root->dump(os, 0);
  }
}

std::ostream& operator<< (std::ostream& os, const JSON& json)
{
  json.dump(os);
  return os;
}


JSON_node::JSON_node(std::ifstream& filehandle) : filehandle(filehandle)
{
  this->skip_whitespace();

  // Set default node type = null
  this->node_type = JSON_nodetype::JSON_nodetype_null;

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
    JSON_node* child = new JSON_node(this->filehandle);

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
  // JSON object key may or may not be quoted

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
  this->objectkeys_index = new JSON_objectkeytype();
  this->node_value.objectvalues_value = new JSON_objectvaluetype();

  this->advance(); // Consume prefix curly
  this->skip_whitespace();
  while(this->peek() != '}') {

    // Consume next item key
    std::string key = find_object_key();

    // Look for colon
    this->consume(':', "Expected : separator after object key '" + key + "'");

    // Consume next item value
    JSON_node* value = new JSON_node(filehandle);

    // Add key/value pair to self
    this->objectkeys_index->push_back( key );
    this->node_value.objectvalues_value->insert({ key, value });

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
  if (this->is_eof()) return 0;
  return this->filehandle.peek();
}


void JSON_node::error(std::string message)
{
  std::string errormsg = message + " near byte " + std::to_string(this->filehandle.tellg()) + ": '" + (const char)this->peek() + "'";
  throw std::runtime_error(errormsg);
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



void JSON_node::dump_prefix(std::ostream& os, uint8_t depth) const
{
  for (uint8_t i=0; i<depth; i++) os << "  ";
}


void JSON_node::dump_string(std::ostream& os, uint8_t depth) const
{
  os << '"' << *this->node_value.string_value << '"';
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
  size_t count = this->objectkeys_index->size();
  size_t index = 0;
  if (count > 0) {
    os << "{" << std::endl;
    for (auto& key : *this->objectkeys_index) {
      this->dump_prefix(os, depth+1);
      os << "\"" << key << "\": ";
      this->node_value.objectvalues_value->at(key)->dump(os, depth+1);
      index++;
      os << (index < count ? "," : "") << std::endl;
    }
    this->dump_prefix(os, depth);
    os << "}";
  } else {
    os << "{}";
  }
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

