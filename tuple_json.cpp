#include <tuple>
#include <iostream>
#include <sstream>
#include <variant>
#include <regex>

using tElementValue = std::variant<int, double, std::string>;

//Task: ostream wrapper for tuple?
//Task: Tuple in Tuple (overload this)
template <typename T1>
constexpr void tuple_element_jsonify(std::ostream& o, int count, const T1& t1) {
    tElementValue v = t1;
    o << "    \"element\": {\n"
      << "      \"value\": \"" << t1 << "\", "
      << "\"type\": \"" <<   v.index() << "\"" << "},\n"; 
}

//Wrapping tuple to allow an streaming operator overload
template<typename T> struct my_tuple 
{
    my_tuple(const T& t) : tu(t) {

        std::cout << "DEBUG ctor from tuple" << std::endl;
    };

    template<typename T2> 
    friend constexpr std::ostream& operator<<(std::ostream& o, const my_tuple<T2>& mt);

private:
    T tu;
};

#define C_ALL(X) cbegin(X), cend(X)
/**
 * Tuple from JSON string utility
*/
struct tuple_from_s 
{
    const std::string ELEMENT = "\"element\"";
    const std::string TUPLE = "\"tuple\"";
    const std::string VALUE = "\"value\"";
    const std::string TYPE = "\"type\"";
    

    //CTOR: Take a JSON string and create our tuple
    //
    //"tuple": {
    //    "element": {
    //        "value": "1", "position": "1", "type": "i"        },   
    tuple_from_s(const std::string& s) : tu() {

        const std::regex re("\"value\": \"([0-9A-Za-z.]*)\", \"type\": \"([0-9]*)\""); 

        tElementValue vt;
        for (std::sregex_iterator it{s.begin(), s.end(), re}; it != std::sregex_iterator{}; ++it)
        {
            const auto& v = (*it);
            std::cout << "GOT: v=" << v[1] << " t=" << v[2] << std::endl;
            int t = stoi(v[2]);
            switch (t)
            {
                case 0:
                    vt.emplace<0>(stoi(v[1]));
                    break;
                case 1:
                    vt.emplace<1>(stod(v[1]));
                    break;
                case 2:
                    vt.emplace<2>(v[1]);
                    break;
                default:
                    std::cout << "ERROR not supported type" << std::endl;
            }

            //make a tuple
        }
    };

private:
    std::tuple<> tu;
};

template<typename T2> 
constexpr std::ostream& operator<<(std::ostream& o, const my_tuple<T2>& mt) {

    o << "{\n"
    << "  \"tuple\": {\n";
    std::apply([&] (const auto&... t) { 
            (( tuple_element_jsonify(o, 1, t) ),...); //FOLD experession using , to chain commands. 
            }, mt.tu);
    o  << "  }\n"
        << "}\n";
    return o;
}

 int main()
 {  
    std::cout << "start" << std::endl;
    my_tuple t1 =  my_tuple(std::make_tuple(1, "two", 3.001, '4'));
    std::cout << t1 << std::endl;
    std::stringstream ss;
    ss << t1;
    std::cout << ss.str() << std::endl;
  
    //tuple from json string
    tuple_from_s t3(ss.str());

    std::cout << "end" << std::endl;
    return 0;
 }