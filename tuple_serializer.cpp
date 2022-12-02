//
// HOMEWORK: implement these two functions below and make them work properly
// - doesn't have to be JSON, you can come up with your own protocol
// - remember "fold expressions" and "pack expansion" -- they might come useful for the variadics

#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <vector>
#include <variant>
#include <cassert>
#include <charconv>
#include <utility>
#include <boost/convert.hpp>
#include <boost/convert/strtol.hpp>

using tNumber = long;
using tBool = bool;
using tDouble = double;
using tString = std::string;

using tTypes = std::variant<tNumber, tDouble, tString, tBool>;

template<int N, typename... Ts> using NthTypeOf =
        typename std::tuple_element_t<N, std::tuple<Ts...>>;

const char DELIM = '\n';
const char NUMBER_TYPE = 'l';
const char DOUBLE_TYPE = 'd';
const char STRING_TYPE = 's';
const char BOOL_TYPE = 'b';

template<typename T>
void print_tuple(std::stringstream& ss, const T& t)
{
    ss << STRING_TYPE << t << DELIM;
}

template<>
void print_tuple(std::stringstream& ss, const tDouble& t)
{
    ss << DOUBLE_TYPE << t << DELIM;
}

template<>
void print_tuple(std::stringstream& ss, const tNumber& t)
{
    ss << NUMBER_TYPE << t << DELIM;
}

template<>
void print_tuple(std::stringstream& ss, const tBool& b)
{
    ss << BOOL_TYPE << ( b ? "1" : "0" ) << DELIM;
}

template <typename... Ts>
std::string serialize_from_tuple(const std::tuple<Ts...>& t)
{
    std::stringstream ss;
    std::apply([&](const auto&... t) 
        {
            ((print_tuple(ss, t)), ...); //FOLD over the param pack
        }, 
        t); //Q: still bit of a mental block about going from tuple<Ts...>  to auto&... t ?
            //is it each param pack?

    return ss.str();
}

/**
 * Test function:  each element in the seralised string matches the strings expected
*/
void test_serialised(const std::string& s, const std::vector<std::string>& expected)
{
    int line = 0;
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = s.find(DELIM, prev)) != std::string::npos) {
        const std::string_view el = s.substr(prev, pos - prev);
        prev = pos + 1;

        std::cout << "Test:" << el << "=" << expected[line] << std::endl;
        assert(el == expected[line]);
        line++;
    }
}

template <typename... Ts>
std::tuple<Ts...> deserialize_into_tuple(const std::string& s)
{
    std::array<tTypes, sizeof...(Ts)> variants; 

    //1. tokenize our string to get type and data, into an array of variants
    {
        size_t left = 0;
        size_t right = 0;
        size_t elementIndex = 0;
        //Tokenize - sequence scan and and create tokens of correct type
        //c++ ranges in c++23
        while ((right = s.find('\n', left)) != std::string::npos) {
            char type = s.at(left);
            const std::string data = s.substr(left+1, (right - (left+1)));
            std::cout << "deserialize_into_tuple: type " << type << " got " << data  << std::endl;

            switch (type)
            {
                case STRING_TYPE:
                    variants[elementIndex++] = data;
                    break;
                case NUMBER_TYPE:
                    variants[elementIndex++] =  atol(data.c_str()); 
                    //string view version could be : boost::convert<long>(data, boost::cnv::strtol()).get();
                    break;
                case DOUBLE_TYPE:
                    variants[elementIndex++] =  atof(data.c_str());
                    break;
                case BOOL_TYPE:
                    variants[elementIndex++] = (data=="true" ? true : false);
                    break;
                default:
                    std::cout << "Error unknown type " << type << std::endl;
                    throw(std::exception());
            };
            left = right + 1;
        }
    }

    //2. Array to Tuple 
    std::tuple<Ts...> tup;

    {
        //need the compile time size of the tuple to convert array to tuple
        auto seq = std::make_index_sequence<sizeof...(Ts)>{}; 

        //do NOT use apply on the index_sequence
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            auto setElem = [](auto& tuple_el, const auto& variant_el) { 
                tuple_el = variant_el;
                };
            ( setElem(std::get<I>(tup), std::get<NthTypeOf<I, Ts...>>(variants[I])),...);
        }(seq);
    }
    return tup;
}

/**
 * Debug function to print a tuple contents to cout
*/
template <typename... Ts>
void tuple_print(const std::tuple<Ts...>& t)
{
    std::apply([](auto&... te) {
        std::cout << "tuple_print: ";
        auto fn = [](auto& el) { std::cout << el << ","; };
        ((fn(te),...));
        std::cout << std::endl;
    }, t);
}

/**
 * MAIN
 */
 int main()
 {  
    std::cout << "start" << std::endl;

    //Test 1
    {
        using tTuple1 = std::tuple<tNumber, tDouble, tString, tBool>;

        const tTuple1 t1a = std::make_tuple(1L, 2.3, std::string("three"), false);
        const std::string s1 = serialize_from_tuple(t1a);

        test_serialised(s1, {"l1","d2.3","sthree","b0"});
        tuple_print(t1a);

        tTuple1 t1b = deserialize_into_tuple<tNumber, tDouble, tString, tBool>(s1);
        tuple_print(t1b);
        assert( t1b == t1a );
    }
    //Test 2: same type in several elements
    {
        using tTuple2 = std::tuple<tNumber, tNumber, tNumber>;

        const tTuple2 t2a = std::make_tuple(12L, 22L, 32L);
        const std::string s2 = serialize_from_tuple(t2a);
        test_serialised(s2, {"l12","l22","l32"});
        tuple_print(t2a);

        tTuple2 t2b = deserialize_into_tuple<tNumber, tNumber, tNumber>(s2);
        tuple_print(t2b);
        assert( t2b == t2a );
        
    }
    std::cout << "end" << std::endl;
 }