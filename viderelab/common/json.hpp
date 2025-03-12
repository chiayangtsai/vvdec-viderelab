
#pragma once

#if !defined(__VIDERELAB_COMMON_JSON_H__)
#define __VIDERELAB_COMMON_JSON_H__

#include <viderelab/common/exception.hpp>

#include <format>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace viderelab {
namespace json {

static constexpr size_t DEFAULT_TAB_STEP{2u};

enum class eFlags : uint32_t
{
    NONE = 0x00,
    FLAT_ARRAY = 0x02,
    FLAT_DICT = 0x04
};

inline
bool isSet(const eFlags flags, const eFlags mask)
{
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(mask)) != 0;
}

inline
eFlags operator & (const eFlags lhs, const eFlags rhs)
{
    return static_cast<eFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

inline
eFlags operator | (const eFlags lhs, const eFlags rhs)
{
    return static_cast<eFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

struct CONTROL
{
    uint32_t tab{DEFAULT_TAB_STEP};
    eFlags flags{eFlags::FLAT_ARRAY};

    std::string GetPrefix(const size_t inc, const bool flat) const
    {
        if (flat) {
            return std::string(" ");
        } else {
            return std::string("\n" + std::string(tab * inc, ' '));
        }
    }
};

enum class eType
{
    None,
    Boolean,
    Integer,
    Float,
    String,
    Array,
    Dict
};

class Value
{
public:
    virtual
    ~Value() = default;

    virtual
    const std::string to_string(const size_t inc = 0u, const CONTROL &ctrl = defaultControl) const = 0;

protected:
    static constexpr CONTROL defaultControl{};
};

#pragma pack(push, 1)

class Null : public Value
{
public:
    Null() = default;
    const std::string to_string(const size_t inc = 0u, const CONTROL &ctrl = defaultControl) const override
    {
        (void) inc, ctrl;
        return "null";
    }
};

class Boolean : public Value
{
public:
    Boolean() = default;
    explicit Boolean(const bool value) : m_value(value) {}
    const std::string to_string(const size_t inc = 0u, const CONTROL &ctrl = defaultControl) const override
    {
        (void) inc, ctrl;
        return m_value ? "true" : "false";
    }

private:
    bool m_value;
};

class Integer : public Value
{
public:
    Integer() = default;
    explicit Integer(const int64_t value) : m_value(value) {}
    const std::string to_string(const size_t inc = 0u, const CONTROL &ctrl = defaultControl) const override
    {
        (void) inc, ctrl;
        return std::to_string(m_value);
    }

private:
    int64_t m_value;
};

class Float : public Value
{
public:
    Float() = default;
    explicit Float(const double value) : m_value(value) {}
    const std::string to_string(const size_t inc = 0u, const CONTROL &ctrl = defaultControl) const override
    {
        (void) inc, ctrl;
        return std::to_string(m_value);
    }

private:
    double m_value;
};

class String : public Value
{
public:
    String() = default;
    explicit String(const std::string& value) : m_value(value) {}
    const std::string to_string(const size_t inc = 0u, const CONTROL &ctrl = defaultControl) const override
    {
        (void) inc, ctrl;
        return std::format("\"{}\"", m_value);
    }

private:
    std::string m_value;
};

template <typename type_t>
bool constexpr is_char_string()
{
    return std::is_same_v<type_t, char*> ||
        std::is_same_v<type_t, const char*> ||
        (std::is_array_v<type_t> &&
            (std::is_same_v<std::remove_extent_t<type_t>, char> ||
             std::is_same_v<std::remove_extent_t<type_t>, const char>));
}

class Dict;
class Array : public Value
{
public:
    Array() :
        m_array{std::make_shared<std::vector<std::shared_ptr<Value>>>()} {}
    Array(const Array& other) :
        m_array(other.m_array) {}

    template <typename type_t>
    void push_back(const type_t& value)
    {
        if constexpr (std::is_convertible_v<type_t, std::shared_ptr<Value>>) {
            auto p{dynamic_cast<Array *>(value.get())};
            if (p == this) {
                throw Exception("Array::push_back: Recursive array");
            }
        } else if constexpr (std::is_same_v<type_t, Array>) {
            if (&value == this) {
                throw Exception("Array::push_back: Recursive array");
            }
        }

        if constexpr (std::is_arithmetic<type_t>::value) {
            if constexpr (std::is_same_v<type_t, bool>) {
                m_array->push_back(std::make_shared<Boolean>(value));

            } else if constexpr (std::is_integral_v<type_t>) {
                m_array->push_back(std::make_shared<Integer>(value));

            } else if constexpr (std::is_floating_point_v<type_t>) {
                m_array->push_back(std::make_shared<Float>(value));

            } else {
                throw Exception("Array::push_back: Unsupported type");
            }
        } else if constexpr (is_char_string<type_t>()) {
            m_array->push_back(std::make_shared<String>(value));

        } else if constexpr (std::is_same_v<type_t, std::string>) {
            m_array->push_back(std::make_shared<String>(value));

        } else if constexpr (std::is_same_v<type_t, std::shared_ptr<Value>>) {
            m_array->push_back(value);

        } else if constexpr (std::is_same_v<type_t, Array>) {
            m_array->push_back(std::make_shared<Array>(value));

        } else if constexpr (std::is_same_v<type_t, Dict>) {
            m_array->push_back(std::make_shared<Dict>(value));

        } else {
            throw Exception("Array::push_back: Unsupported type");
        }
    }

    template <typename type_t>
    void append(const std::span<type_t> value)
    {
        for (const auto& v : value) {
            push_back(v);
        }
    }

    template <typename type_t>
    void push_back(const std::span<type_t> value)
    {
        Array arr;
        for (const auto& v : value) {
            arr.push_back(v);
        }
        push_back(arr);
    }

    const std::string to_string(const size_t inc = 0u, const CONTROL &ctrl = defaultControl) const
    {
        if (m_array->empty()) {
            return "[]";
        }

        const std::string prefix{ctrl.GetPrefix(inc + 1u, isSet(ctrl.flags, eFlags::FLAT_ARRAY))};

        const CONTROL overrideCtrl{
            .tab = ctrl.tab,
            .flags = isSet(ctrl.flags, eFlags::FLAT_ARRAY) ? (eFlags::FLAT_ARRAY | eFlags::FLAT_DICT) : ctrl.flags
        };

        std::string str;
        str += "[";
        for (const auto& value : *m_array) {
            if (&value != &m_array->front()) {
                str += ",";
            }
            str += prefix + value->to_string(inc + 1u, overrideCtrl);
        }

        const std::string suffix{ctrl.GetPrefix(inc, isSet(ctrl.flags, eFlags::FLAT_ARRAY))};
        str += suffix + "]";
        return str;
    }

private:
    std::shared_ptr<std::vector<std::shared_ptr<Value>>> m_array;
};

class Dict : public Value
{
public:
    Dict() :
        m_keys{std::make_shared<std::set<std::string_view>>()},
        m_dict{std::make_shared<std::list<pair_t>>()} {}
    Dict(const Dict& other) :
        m_keys(other.m_keys),
        m_dict(other.m_dict) {}

    template <typename type_t>
    void insert(const std::string& key, const type_t& value)
    {
        if (m_keys->find(key) != m_keys->end()) {
            throw Exception(std::format("Dict::insert: Key '{}' already exists", key));
        }

        if constexpr (std::is_convertible_v<type_t, std::shared_ptr<Value>>) {
            auto p{dynamic_cast<Dict *>(value.get())};
            if (p == this) {
                throw Exception("Dict::insert: Recursive dictionary");
            }
        } else if constexpr (std::is_same_v<type_t, Dict>) {
            if (&value == this) {
                throw Exception("Dict::insert: Recursive dictionary");
            }
        }

        if constexpr (std::is_arithmetic<type_t>::value) {
            if constexpr (std::is_same_v<type_t, bool>) {
                m_dict->push_back(pair_t{key, std::make_shared<Boolean>(value)});

            } else if constexpr (std::is_integral_v<type_t>) {
                m_dict->push_back(pair_t{key, std::make_shared<Integer>(value)});

            } else if constexpr (std::is_floating_point_v<type_t>) {
                m_dict->push_back(pair_t{key, std::make_shared<Float>(value)});

            } else {
                throw Exception("Dict::insert: Unsupported type");
            }
        } else if constexpr (is_char_string<type_t>()) {
            m_dict->push_back(pair_t{key, std::make_shared<String>(value)});

        } else if constexpr (std::is_same_v<type_t, std::string>) {
            m_dict->push_back(pair_t{key, std::make_shared<String>(value)});

        } else if constexpr (std::is_convertible_v<type_t, std::shared_ptr<Value>>) {
            m_dict->push_back(pair_t{key, value});

        } else if constexpr (std::is_same_v<type_t, Array>) {
            m_dict->push_back(pair_t{key, std::make_shared<Array>(value)});

        } else if constexpr (std::is_same_v<type_t, Dict>) {
            m_dict->push_back(pair_t{key, std::make_shared<Dict>(value)});

        } else {
            throw Exception("Dict::insert: Unsupported type");
        }
        m_keys->insert(m_dict->back().first);
    }

    template <typename type_t>
    void insert(const std::string& key, const std::span<type_t> value)
    {
        Array arr;
        for (const auto& v : value) {
            arr.push_back(v);
        }
        insert(key, arr);
    }

    template <typename type_t>
    void insert(const std::string& key, const std::optional<type_t>& value)
    {
        if (value.has_value()) {
            insert(key, value.value());
        } else {
            throw Exception("Dict::insert: Optional has no value");
        }
    }

    const std::string to_string(const size_t inc = 0u, const CONTROL &ctrl = defaultControl) const
    {
        if (m_dict->empty()) {
            return "{}";
        }

        const std::string prefix{ctrl.GetPrefix(inc + 1u, isSet(ctrl.flags, eFlags::FLAT_DICT))};

        std::string str;
        str += "{";
        for (const auto& [key, value] : *m_dict) {
            if (&value != &m_dict->front().second) {
                str += ",";
            }
            str += prefix + "\"" + key + "\": " + value->to_string(inc + 1u, ctrl);
        }

        const std::string suffix{ctrl.GetPrefix(inc, isSet(ctrl.flags, eFlags::FLAT_DICT))};
        str += suffix + "}";
        return str;
    }

private:
    using pair_t = std::pair<std::string, std::shared_ptr<Value>>;

    std::shared_ptr<std::set<std::string_view>> m_keys;
    std::shared_ptr<std::list<pair_t>> m_dict;
};

#pragma pack(pop)

class Writer
{
public:

private:
};

} // namespace json
} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_JSON_H__)
