#include "string_rope.h"

#include <iostream>

string_rope::string_rope(const char *string) { rope.push_back(string); }
string_rope::string_rope(std::string string) {
  rope.push_back(std::move(string));
}
string_rope &string_rope::operator+(std::string string) {
  rope.push_back(std::move(string));
  return *this;
}
string_rope &string_rope::operator+(const string_rope &otherRope) {
  rope.insert(rope.end(), otherRope.rope.begin(), otherRope.rope.end());
  return *this;
}
string_rope &string_rope::operator+(string_rope &&otherRope) {
  rope.insert(rope.end(), std::make_move_iterator(otherRope.rope.begin()),
              std::make_move_iterator(otherRope.rope.end()));
  return *this;
}
string_rope &string_rope::operator+=(std::string string) {
  rope.push_back(std::move(string));
  return *this;
}
string_rope &string_rope::operator+=(const char *string) {
  rope.push_back(std::move(string));
  return *this;
}
string_rope &string_rope::operator+=(const string_rope &otherRope) {
  rope.insert(rope.end(), otherRope.rope.begin(), otherRope.rope.end());
  return *this;
}
string_rope &string_rope::operator+=(string_rope &&otherRope) {
  rope.insert(rope.end(), std::make_move_iterator(otherRope.rope.begin()),
              std::make_move_iterator(otherRope.rope.end()));
  return *this;
}

bool string_rope::empty() const { return rope.empty(); }
size_t string_rope::size() const {
  size_t size = 0;
  for (const auto &segment : rope) {
    size += segment.size();
  }
  return size;
}
std::string string_rope::toString() const {
  std::string string;
  string.reserve(size());
  for (const auto &segment : rope) {
    string += segment;
  }
  return string;
}

std::ostream &operator<<(std::ostream &os, const string_rope &rope) {
  for (auto const &string : rope.rope) {
    os << string;
  }
  return os;
}
