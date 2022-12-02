/*
* HOMEWORK (21Nov22) Create a JsonObject class
*
**/
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <variant>
#include <sstream>

using tName = std::string;
using tElementPos = long;

using tElementString = std::string;
using tElementNumber = long;
using tElementBool = bool;
using tElement = std::variant<tElementNumber, tElementString, tElementBool>;

//Helper to visit variant and return string from element
struct StringifyVistor
{
    std::string operator()(const tElementNumber x)
    {
        std::stringstream ss;
        ss << x;
        return ss.str();
    }
    std::string operator()(const tElementBool x)
    {
        return std::string((x ? "true" : "false"));
    }
    std::string operator()(const tElementString& x)
    {
        std::stringstream ss;
        ss << '"' << x << '"';
        return ss.str();
    }
    
};

/**
 * Json Object
*/
class JsonObject
{

public:

    JsonObject() {}

    JsonObject(std::initializer_list<std::pair<tName, tElement>>&& il)
    {
        //Talk about the move semantics here..  does this work? did i need && at all points?
        for ( auto&& p : il )
        {
            elements.insert(p);  //don't need std::move right?
        }
    }
    template<typename T>
    void add(const tName& name, const T& value)
    {
        //std::cout << "add typeid=" << typeid(T).name() << std::endl;
        // not working.. compile eror with string type
        //tElement el;
        //std::get<T>(el) = value;
        elements.insert({name, tElement{value}});
    }

    //Get elemnt by position
    std::string operator[](const tName& name) const
    {
        return std::visit(StringifyVistor{}, elements.at(name));
    }

    //ostream support
    friend std::ostream& operator<<(std::ostream& o, const JsonObject& j);

private:
    std::map<tName, tElement> elements;
};

//Friend of JsonObject
std::ostream& operator<<(std::ostream& o, const JsonObject& j)
{
    auto vistor = StringifyVistor{};
    o << "jsonObject: {\n";
    for( auto [k,v] : j.elements )
    {
        std::cout << '\t' << k << ": " << std::visit(vistor, v) << "," << std::endl;
    }
    o << "}\n";
    return o;
}

/**
 * main
*/
int main()
{
    {
        std::cout << "\nTest A:" << std::endl;
        JsonObject a;
        a.add("myelement0", 123L);
        a.add("myelement1", 456L);
        a.add("myelement2", std::string{"myvalue1"});
        a.add("myelement3", false);

        std::cout <<  a["myelement0"] << std::endl;
        std::cout <<  a["myelement1"] << std::endl;
        std::cout <<  a["myelement2"] << std::endl;
        std::cout <<  a["myelement3"] << std::endl;
        std::cout <<  a << std::endl;
    }
    {
        std::cout << "\nTest B:" << std::endl;
        JsonObject b = {
            //{"happy", "yes"},
            {"mylong", 1234L},
            {"mybool", false},
            {"mystring", "test"},
            {"mystring2", "test2"},
        };
        std::cout <<  "Test B:" << b << std::endl;
    }
}

//
// BONUS HOMEWORK: implement this function below
// - let's limit it to one-level JSON, no recursion or arrays 

//std::string deserialize_from_json(const JsonObject& obj);
//JsonObject serialize_to_json(const std::string& s);
//
// JsonObject[0]