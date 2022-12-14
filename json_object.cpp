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
#include <cassert>

using tName = std::string;
using tElementPos = long;

using tElementString = std::string;
using tElementNumber = long;
using tElementBool = double;

//HOMEWORK  contains another sequence of tElements?
// struct with indirection (e.g pointer)
// recursion in the visitor more complex

/**
 * Json Object
*/
class JsonObject
{
    struct tElement;
    using tElements = std::map<tName, tElement>;

    /*
    * tElement node storage
    */
    struct tElement {
        using tElementData = std::variant<tElementNumber, tElementString, tElementBool, tElements>;

        template<typename T>
        tElement(const T& x) {
            node = x;
        }        

        tElement(std::initializer_list<std::pair<tName, tElement>> il) {
            tElements t;
            for ( const auto& p : il )
                t.insert(p); 
            node = t; 
        }   

        tElement& operator[](const tName& name) {
            if (std::holds_alternative<tElements>(node)) {
                return std::get<tElements>(node).at(name);
            }
            throw std::invalid_argument(name); //empty/null or error?
        }

        /*bool operator==(const tElement& rhs) {
            return node == rhs;
        }*/
        tElementData node;
    };

    //Helper to visit variant and return string from element
    struct StringifyVistor {
        std::string operator()(const tElementNumber x) const {
            return std::to_string(x);
        }
        std::string operator()(const tElementBool x) const {
            return std::string((x ? "true" : "false"));
        }
        std::string operator()(const tElementString& x) const {
            std::stringstream ss;
            ss << '"' << x << '"';
            return ss.str();
        }
        std::string operator()(const tElements& x) const {
            std::stringstream ss;
            ss << "{ ";
            for( const auto& [k,v] : x )
                ss  << k << ": " << std::visit(JsonObject::vistor, v.node) << ", ";
            ss << "}";
            return ss.str();
        }
    };
public:
    static constexpr auto vistor = StringifyVistor{};

    JsonObject() {}

    JsonObject(std::initializer_list<std::pair<tName, tElement>> il)  { //best practice  to pass IL by value
        for ( const auto& p : il )
            elements.insert(p);  
    }

    template<typename T>
    void add(const tName& name, T&& value) { //&& forwarding reference because T is template type
        elements.insert({name, tElement{std::forward<T>(value)}});
    }

    //Get element for edit print
    tElement& operator[](const tName& name) {
        return elements.at(name);
    }

    const tElement& operator[](const tName& name) const {
        return elements.at(name);
    }

    //ostream support friends
    friend std::ostream& operator<<(std::ostream& o, const JsonObject& j);
    friend std::ostream& operator<<(std::ostream& o, const tElement& j);

private:
    tElements elements;
};

//Friend of JsonObject
std::ostream& operator<<(std::ostream& o, const JsonObject& j) {
    o << "jsonObject: {";
    for( const auto& [k,v] : j.elements )
    {
        o  << k << ": " << std::visit(JsonObject::vistor, v.node) << ", ";
    }
    o << "}\n";
    return o;
}

//Friend of JsonObject
std::ostream& operator<<(std::ostream& o, const JsonObject::tElement& e) {
    o << std::visit(JsonObject::vistor, e.node);
    return o;
}

/**
 * main
*/
int main() {

    //Test A
    std::cout << "\n*** Test A - add, alter, print" << std::endl;
    {
        
        JsonObject a;
        a.add("myelement0", 123L);
        a.add("myelement1", 456L);
        a.add("myelement2", std::string{"myvalue1"});
        a.add("myelement3", false);

        std::cout << "Lookup with []: " << a["myelement0"] << "," << a["myelement1"] << ","
            << a["myelement2"] <<  "," << a["myelement3"] << std::endl;

        std::cout << "Json object to string: " << a << std::endl;

        std::cout << "Alter elements:" << std::endl;
        a["myelement3"] = 10L;
        a["myelement0"] = "convert";
        std::cout <<  a << std::endl;

        auto s = std::string{"myvalue1"}; 
        a.add("myelement4", s); //test L-value passing to move 
    }

    //Test B
    std::cout << "\n*** Test B - initialiser list ctor" << std::endl;
    {
        JsonObject b = {
            {"mylong", 1234L}, {"mybool", false}, {"mystring", "test"}, {"mystring2", "test2"} 
        };
        std::cout <<  "Json object to string: " << b << std::endl;
    }

    //Test C
    std::cout << "\n*** Test C - deeper structures" << std::endl;
    {
        //construct with sub-elements
        JsonObject c = {
            {"mylong", 1234L}, 
            { "subnode1", { { "sub1", 1L }, { "sub2", "blah" }, { "sub3", false } } },
            {"mybool", false},
        };

        //access sub-elements
        std::cout << "Get subelement using []: " << c["subnode1"]["sub1"] << std::endl;
        std::cout << "Get subelement using []: " << c["subnode1"]["sub2"] << std::endl;
        
        //assert(c["subnode1"]["sub2"] == "blah"); can't compare yet

        //update sub-elements
        c["subnode1"]["sub3"] = true;
        std::cout << "Get updated subelement using []: " << c["subnode1"]["sub3"] << std::endl; //BUG returning 1 not true

        //TODO can i add structured nodes?
        //c.add({ "subnode1", { { "subelement1", 1L }, { "subelement2", false } });

        std::cout <<  "Json object to string: " << c << std::endl;
    }   
}
