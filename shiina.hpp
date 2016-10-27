#ifndef SHIINA_HPP
#define SHIINA_HPP

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <fstream>
#include <iostream>

namespace shiina_detail {

void trim(std::string& input) {
    auto start = std::begin(input);
    while (std::isspace(*start))
        start++;
    auto last = std::end(input);
    while (std::isspace(*(last-1)))
        last--;
    input = std::string{start, last};
}

}


struct shiina {
    enum class Type {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    };

    using Array = std::vector<shiina>;
    using Object = std::map<std::string, shiina>;

    shiina() :
        m_type(Type::Null)
    {}
    shiina(Type type) :
        m_type(type)
    {
        switch (type) {
        case Type::Null:
            null = nullptr;
            break;
        case Type::Boolean:
            boolean = false;
            break;
        case Type::Number:
            number = 0.0;
            break;
        case Type::String:
            assign("");
            break;
        case Type::Array:
            assign(Array{});
            break;
        case Type::Object:
            assign(Object{});
            break;
        }
    }
    explicit shiina(bool b) :
        m_type(Type::Boolean)
    {
        boolean = b;
    }
    explicit shiina(double num) :
        m_type(Type::Number)
    {
        number = num;
    }
    explicit shiina(Array const& arr) :
        m_type(Type::Array)
    {
        assign(arr);
    }
    explicit shiina(Object const& obj) :
        m_type(Type::Object)
    {
        assign(obj);
    }
    explicit shiina(std::string const& str) :
        m_type(Type::String)
    {
        assign(str);
    }
    explicit shiina(const char* s) :
        m_type(Type::String)
    {
        assign(std::string{s});
    }

    shiina(shiina const& rhs) :
        m_type(rhs.m_type)
    {
        switch (m_type) {
        case Type::Null:
            null = nullptr;
            break;
        case Type::Array:
            assign(rhs.as_array());
            break;
        case Type::Object:
            assign(rhs.as_object());
            break;
        case Type::String:
            assign(rhs.as_string());
            break;
        case Type::Number:
            number = rhs.as_number();
            break;
        case Type::Boolean:
            boolean = rhs.as_boolean();
            break;
        }
    }
    shiina(shiina&& rhs) :
        m_type(rhs.m_type)
    {
        switch (m_type) {
        case Type::Null:
            null = nullptr;
            break;
        case Type::Array:
            assign(std::move(rhs.as_array()));
            break;
        case Type::Object:
            assign(std::move(rhs.as_object()));
            break;
        case Type::String:
            assign(std::move(rhs.as_string()));
            break;
        case Type::Number:
            number = rhs.as_number();
            break;
        case Type::Boolean:
            boolean = rhs.as_boolean();
            break;
        }
    }

    ~shiina() {
        clear();
    }

    shiina& operator=(shiina const& rhs);
    shiina& operator=(shiina&& rhs);

    struct parsing_error {
        std::string message;
    };

    static void check(std::string& input, char c) {
        shiina_detail::trim(input);
        if (input[0] != c)
            throw parsing_error{"missing char"};
        input.erase(std::begin(input));
        shiina_detail::trim(input);
    }

    static shiina parse_number(std::string& input) {
        shiina_detail::trim(input);
        size_t i = 0;
        while (('0' <= input[i] && input[i] <= '9') || input[i] == '.')
            i++;
        double res = std::stod(input.substr(0, i));
        input.erase(0, i);
        return shiina{res};
    }

    static shiina parse_string(std::string& input) {
        check(input, '"');
        std::string res;
        auto it = std::begin(input);
        while (*it != '"') {
            res += *it;
            it++;
        }
        input.erase(0, res.size());
        check(input, '"');
        return shiina{res};
    }

    static shiina parse_array(std::string& input) {
        check(input, '[');
        shiina result{Type::Array};
        while (true) {
            shiina_detail::trim(input);
            result.as_array().push_back(shiina::parse(input));
            char c = input[0];
            input.erase(std::begin(input));
            if (c == ',')
                continue;
            if (c == ']')
                break;
        }
        return result;
    }

    static std::string parse_identifier(std::string& input) {
        check(input, '"');
        std::string res;
        auto it = std::begin(input);
        while (*it != '"') {
            res += *it;
            it++;
        }
        input.erase(0, res.size());
        check(input, '"');
        return res;
    }
    static shiina parse_object(std::string& input) {
        shiina::check(input, '{');
        shiina result{Type::Object};
        while (true) {
            shiina_detail::trim(input);
            auto label = shiina::parse_identifier(input);
            shiina::check(input, ':');
            shiina_detail::trim(input);
            shiina obj = shiina::parse(input);
            shiina_detail::trim(input);
            result.as_object().emplace(label, obj);
            char c = input[0];
            input.erase(std::begin(input));
            if (c == ',')
                continue;
            if (c == '}')
                break;
        }
        return result;
    }
    static shiina parse(std::string const& input) {
        std::string str = input;
        return shiina::parse(str);
    }
    static shiina parse(std::string& input) {
        shiina_detail::trim(input);
        switch (input[0]) {
        case '[':
            return parse_array(input);
        case '{':
            return parse_object(input);
        case '\"':
            return parse_string(input);
        default:
            if ('0' <= input[0] && input[0] <= '9')
                return parse_number(input);
            if (input.substr(0, 4) == "null") {
                input.erase(0, 4);
                return shiina{nullptr};
            } else if (input.substr(0, 4) == "true") {
                input.erase(0, 4);
                return shiina{true};
            } else if (input.substr(0, 5) == "false") {
                input.erase(0, 5);
                return shiina{false};
            } else
                throw parsing_error{input};
        }
    }

    // TODO: check value type
    std::nullptr_t& as_null() { return null; }
    std::nullptr_t const& as_null() const { return null; }
    bool& as_boolean() { return boolean; }
    bool const& as_boolean() const { return boolean; }
    double& as_number() { return number; }
    double const& as_number() const { return number; }
    std::string& as_string() { return str; }
    std::string const& as_string() const { return str; }
    Array& as_array() { return array; }
    Array const& as_array() const { return array; }
    Object& as_object() { return object; }
    Object const& as_object() const { return object; }

    struct bad_path {
        std::string path;
    };
    shiina& operator[](std::string const& label) {
        auto it = as_object().find(label);
        if (it == std::end(as_object()))
            throw bad_path{label};
        return it->second;
    }
    shiina const& operator[](std::string const& label) const {
        auto it = as_object().find(label);
        if (it == std::end(as_object()))
            throw bad_path{label};
        return it->second;
    }
    // TODO: remove
    Type type() const {
        return m_type;
    }

    static shiina load(const char* filename) {
        std::ifstream ifs(filename);
        if (ifs.fail())
            throw std::string{"file not exist: "} + filename;
        std::string src{
            std::istreambuf_iterator<char>(ifs),
            std::istreambuf_iterator<char>()
        };
        return shiina::parse(src);
    }
private:
    union {
        std::nullptr_t null;
        bool boolean;
        double number;
        std::string str;
        Array array;
        Object object;
    };
    void assign(std::string const& src) {
        new (&str) std::string{src};
    }
    void assign(std::string&& src) {
        new (&str) std::string{std::move(src)};
    }
    void assign(Array const& arr) {
        new (&array) Array{arr};
    }
    void assign(Array&& arr) {
        new (&array) Array{std::move(arr)};
    }
    void assign(Object const& obj) {
        new (&object) Object{obj};
    }
    void assign(Object&& obj) {
        new (&object) Object{std::move(obj)};
    }

    void clear() {
        switch (m_type) {
        case Type::Null:
        case Type::Boolean:
        case Type::Number:
            break;
        case Type::String:
            str.~basic_string();
            break;
        case Type::Array:
            array.~vector();
            break;
        case Type::Object:
            object.~map();
            break;
        }
    }
    Type m_type;
};

inline std::ostream& operator<<(std::ostream& os, shiina const& s) {
    switch (s.type()) {
    case shiina::Type::Null:
        os << "null";
        break;
    case shiina::Type::Boolean:
        os << std::boolalpha << s.as_boolean();
        break;
    case shiina::Type::Number:
        os << s.as_number();
        break;
    case shiina::Type::String:
        os << '"' << s.as_string() << '"';
        break;
    case shiina::Type::Array:
        os << '[';
        for (auto const& e: s.as_array())
            os << e << ',';
        os << ']';
        break;
    case shiina::Type::Object:
        os << '{';
        for (auto const& e: s.as_object())
            os << e.first << ": " << e.second;
        os << '}';
        break;
    }
    return os;
}

#endif
