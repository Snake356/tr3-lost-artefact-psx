// A small JSON reader for the builder's tables (known_inputs.json, levels.json, ...).
#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bytes.h"

class Json {
public:
    enum Type { Null, Bool, Number, String, Array, Object };

    static Json parse(const std::string &text);

    Type type() const { return type_; }
    bool is_object() const { return type_ == Object; }
    bool is_string() const { return type_ == String; }
    bool has(const std::string &key) const { return type_ == Object && obj_.count(key) != 0; }
    const Json &operator[](const std::string &key) const;      // throws when missing
    const Json &operator[](size_t i) const;
    size_t size() const { return type_ == Array ? arr_.size() : obj_.size(); }
    const std::string &str() const;
    double num() const;
    long long integer() const { return static_cast<long long>(num()); }
    bool boolean() const { return b_; }
    const std::vector<Json> &items() const { return arr_; }
    const std::map<std::string, Json> &members() const { return obj_; }
    const std::vector<std::string> &keys() const { return order_; }   // members in file order

private:
    Type type_ = Null;
    bool b_ = false;
    double n_ = 0;
    std::string s_;
    std::vector<Json> arr_;
    std::map<std::string, Json> obj_;
    std::vector<std::string> order_;
    friend class JsonParser;
};
