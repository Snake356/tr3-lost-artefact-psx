#include "json.h"

#include <cstdlib>

class JsonParser {
public:
    explicit JsonParser(const std::string &t) : t_(t) {}

    Json value() {
        ws();
        if (p_ >= t_.size()) fail();
        char c = t_[p_];
        Json j;
        if (c == '{') {
            j.type_ = Json::Object;
            p_++;
            ws();
            if (peek() == '}') { p_++; return j; }
            for (;;) {
                ws();
                std::string k = string();
                ws();
                expect(':');
                if (!j.obj_.count(k)) j.order_.push_back(k);
                j.obj_[k] = value();
                ws();
                if (peek() == ',') { p_++; continue; }
                expect('}');
                return j;
            }
        }
        if (c == '[') {
            j.type_ = Json::Array;
            p_++;
            ws();
            if (peek() == ']') { p_++; return j; }
            for (;;) {
                j.arr_.push_back(value());
                ws();
                if (peek() == ',') { p_++; continue; }
                expect(']');
                return j;
            }
        }
        if (c == '"') {
            j.type_ = Json::String;
            j.s_ = string();
            return j;
        }
        if (t_.compare(p_, 4, "true") == 0) { p_ += 4; j.type_ = Json::Bool; j.b_ = true; return j; }
        if (t_.compare(p_, 5, "false") == 0) { p_ += 5; j.type_ = Json::Bool; return j; }
        if (t_.compare(p_, 4, "null") == 0) { p_ += 4; return j; }
        char *end;
        j.n_ = std::strtod(t_.c_str() + p_, &end);
        if (end == t_.c_str() + p_) fail();
        p_ = size_t(end - t_.c_str());
        j.type_ = Json::Number;
        return j;
    }

    void finish() {
        ws();
        if (p_ != t_.size()) fail();
    }

private:
    const std::string &t_;
    size_t p_ = 0;

    [[noreturn]] void fail() { throw Error("bad JSON at byte " + std::to_string(p_)); }
    char peek() { return p_ < t_.size() ? t_[p_] : '\0'; }
    void ws() { while (p_ < t_.size() && (t_[p_] == ' ' || t_[p_] == '\n' || t_[p_] == '\r' || t_[p_] == '\t')) p_++; }
    void expect(char c) {
        if (peek() != c) fail();
        p_++;
    }
    void utf8(std::string &s, uint32_t c) {
        if (c < 0x80) s += char(c);
        else if (c < 0x800) s += char(0xC0 | c >> 6), s += char(0x80 | (c & 0x3F));
        else if (c < 0x10000) s += char(0xE0 | c >> 12), s += char(0x80 | ((c >> 6) & 0x3F)), s += char(0x80 | (c & 0x3F));
        else s += char(0xF0 | c >> 18), s += char(0x80 | ((c >> 12) & 0x3F)), s += char(0x80 | ((c >> 6) & 0x3F)), s += char(0x80 | (c & 0x3F));
    }
    uint32_t hex4() {
        if (p_ + 4 > t_.size()) fail();
        uint32_t v = uint32_t(std::strtoul(t_.substr(p_, 4).c_str(), nullptr, 16));
        p_ += 4;
        return v;
    }
    std::string string() {
        expect('"');
        std::string s;
        for (;;) {
            if (p_ >= t_.size()) fail();
            char c = t_[p_++];
            if (c == '"') return s;
            if (c != '\\') { s += c; continue; }
            char e = t_[p_++];
            switch (e) {
            case 'n': s += '\n'; break;
            case 't': s += '\t'; break;
            case 'r': s += '\r'; break;
            case 'b': s += '\b'; break;
            case 'f': s += '\f'; break;
            case 'u': {
                uint32_t c1 = hex4();
                if (c1 >= 0xD800 && c1 < 0xDC00 && t_.compare(p_, 2, "\\u") == 0) {
                    p_ += 2;
                    uint32_t c2 = hex4();
                    c1 = 0x10000 + ((c1 - 0xD800) << 10) + (c2 - 0xDC00);
                }
                utf8(s, c1);
                break;
            }
            default: s += e;
            }
        }
    }
};

Json Json::parse(const std::string &text) {
    JsonParser p(text);
    Json j = p.value();
    p.finish();
    return j;
}

const Json &Json::operator[](const std::string &key) const {
    auto it = obj_.find(key);
    if (type_ != Object || it == obj_.end()) throw Error("missing \"" + key + "\" in a table");
    return it->second;
}

const Json &Json::operator[](size_t i) const {
    if (type_ != Array || i >= arr_.size()) throw Error("table index out of range");
    return arr_[i];
}

const std::string &Json::str() const {
    if (type_ != String) throw Error("a table value is not a string");
    return s_;
}

double Json::num() const {
    if (type_ != Number) throw Error("a table value is not a number");
    return n_;
}
