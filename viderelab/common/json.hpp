
#pragma once

#if !defined(__VIDERELAB_COMMON_JSON_H__)
#define __VIDERELAB_COMMON_JSON_H__

#include <cstdint>
#include <iostream>
#include <string>

namespace viderelab {
namespace json {

constexpr size_t TAB_INC{2u};

class Item
{
public:
    enum class eAction : uint32_t {
        NONE, // print a regular value
        CLOSING // print the closing bracket
    };

protected:
    Item(std::ostream &s, const size_t tab) :
        m_s(s),
        m_tab(tab) {}

    inline
    std::ostream &s() const {
        return m_s;
    }

    inline
    size_t Tab() const {
        return m_tab;
    }

    inline
    size_t NumValues() const {
        return m_numValues;
    }

    inline
    void IncNumValues() {
        ++m_numValues;
    }

private:
    std::ostream &m_s;
    const size_t m_tab{};
    size_t m_numValues{0u};
};

class Dict : public Item
{
public:
    class Array;

    Dict(std::ostream &s, const size_t tab = TAB_INC) :
        Item(s, tab)
    {
        Item::s() << "{";
    }

    Dict(const Dict &parent) :
        Dict(parent.s(), parent.Tab() + TAB_INC) {}

    Dict(const Array &parent) :
        Dict(parent.s(), parent.Tab() + TAB_INC) {}

    ~Dict()
    {
        PrintTab(Item::Tab() - TAB_INC, eAction::CLOSING);
        Item::s() << "}";
    }

    void StartItem(const std::string &name)
    {
        PrintTab(Item::Tab());
        Item::s() << "\"" + name + "\"" << " : ";
    }

    template <typename T>
    void AddValue(const std::string &name, const T value) 
    {
        StartItem(name);
        Item::s() << value;
    }

    void AddValue(const std::string &name, const std::string &value)
    {
        StartItem(name);
        Item::s() << "\"" << value << "\"";
    }

    Dict StartDict(const std::string &name)
    {
        StartItem(name);
        return Dict(*this);
    }

    Array StartArray(const std::string &name)
    {
        StartItem(name);
        return Array(*this);
    }

    class Array : public Item
    {
    protected:
        friend class Dict;

        Array(std::ostream &s, const size_t tab) :
            Item(s, tab)
        {
            Item::s() << "[";
        }

    public:
        Array(const Dict &parent) :
            Array(parent.s(), parent.Tab() + TAB_INC) {};

        Array(const Array &parent) :
            Array(parent.s(), parent.Tab() + TAB_INC) {};

        ~Array()
        {
            PrintTab(Item::Tab() - TAB_INC, eAction::CLOSING);
            Item::s() << "]";
        }

        void StartItem()
        {
            PrintTab(Item::Tab());
        }

        template <typename T>
        void AddValue(const T value)
        {
            StartItem();
            Item::s() << value;
        }

        Dict StartDict()
        {
            StartItem();
            return Dict(*this);
        }

    private:

        void PrintTab(const size_t tab, const eAction action = eAction::NONE)
        {
            if ((eAction::NONE == action) && (Item::NumValues())) {
                Item::s() << ",";
            }
            Item::s() << std::endl;
            for (size_t i{0u}; i < tab; ++i) {
                Item::s() << " ";
            } 

            Item::IncNumValues();
        }

        Array& operator=(Array const&) = delete;
        Array(Array&&) noexcept = delete;
        Array& operator=(Array&&) noexcept = delete;
    };

protected:

    friend class Array;

private:
    void PrintTab(const size_t tab, const eAction action = eAction::NONE)
    {
        if ((eAction::NONE == action) && (Item::NumValues())) {
            Item::s() << ",";
        }
        Item::s() << std::endl;
        for (size_t i{0u}; i < tab; ++i) {
            Item::s() << " ";
        } 

        Item::IncNumValues();
    }

    Dict& operator=(Dict const&) = delete;
    Dict(Dict&&) noexcept = delete;
    Dict& operator=(Dict&&) noexcept = delete;

};

using Array = Dict::Array;

} // namespace json

} // namespace viderelab

#endif // !defined(__VIDERELAB_COMMON_JSON_H__)
