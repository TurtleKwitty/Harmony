#ifndef string_rope_h
#define string_rope_h

#include <string>
#include <vector>

class string_rope {
public:
  string_rope() = default;
  string_rope(const char *string);
  string_rope(std::string string);
  string_rope &operator+(std::string string);
  string_rope &operator+(const string_rope &otherRope);
  string_rope &operator+(string_rope &&otherRope);
  string_rope &operator+=(std::string string);
  string_rope &operator+=(const char *string);
  string_rope &operator+=(const string_rope &otherRope);
  string_rope &operator+=(string_rope &&otherRope);

  bool empty() const;
  size_t size() const;
  std::string toString() const;

  friend std::ostream &operator<<(std::ostream &os, const string_rope &rope);

  std::vector<std::string> rope;
};

#endif
