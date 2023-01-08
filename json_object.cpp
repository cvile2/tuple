/*
* JsonObject
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
using tElementBool = bool;

/**
 * Json Object Class
*/
class JsonObject {

    struct tElement;
    using tElements = std::map<tName, tElement>;

    /*
    * tElement node storage
    */
    struct tElement { //FF collapse JsonObject and tElement
        using tElementData = std::variant<tElementNumber, tElementString, tElementBool, tElements>;

        //Construct from { x : y,  z : { a: b} } format
        tElement(std::initializer_list<std::pair<tName, tElement>> il) : node{ std::in_place_type<tElements> }  {
            tElements& t = std::get<tElements>(node);       
            for ( const auto& p : il )
                t.insert(p); 
        }   

        template<typename T>
        tElement(const T& x) : node(x) {}  //forward this? not reference 

        tElement& operator[](const tName& name) {
            if (std::holds_alternative<tElements>(node)) {
                return std::get<tElements>(node).at(name);
            }
            throw std::invalid_argument(name); //empty/null or error?
        }

        tElementData node;
    };

    /**
     * Helper to visit variant and return string from element
     */
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
            for( bool first{true}; const auto& [k,v] : x ) { //c++20 init stateuemnt
                ss  << (first ? first=false, " " : ", ") << k << ": " << std::visit(*this, v.node); 
            }
            ss << " }";
            return ss.str();
        }
    };
public:
    JsonObject() {}

    JsonObject(std::initializer_list<std::pair<tName, tElement>> il) { //Note best practice  to pass IL by value
        for ( const auto& p : il )
            elements.insert(p);  
    }

    void add(std::initializer_list<std::pair<tName, tElement>> il) { 
        for ( const auto& p : il )
            elements.insert(p);  
    }

    void add(const tName& name,std::initializer_list<std::pair<tName, tElement>> il) { 
        tElements e;
        for ( const auto& p : il )
            e.insert(p);  
        elements.emplace(name, std::move(e));
    }

    template<typename T>
    void add(const tName& name, T&& value) { //Note && is forwarding reference because T is template type
        elements.emplace(name, tElement{std::forward<T>(value)}); //avoiding copy / move - emplace directly into right memory location
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
    for( bool first{true}; const auto& [k,v] : j.elements ) {
        o  << (first ? first=false, " " : ", ") << k << ": " << std::visit(JsonObject::StringifyVistor{}, v.node);
    }
    o << " }";
    return o;
}

//Friend of JsonObject
std::ostream& operator<<(std::ostream& o, const JsonObject::tElement& e) {
    o << std::visit(JsonObject::StringifyVistor{}, e.node);
    return o;
}

/**
 * main
*/
int main() {

    //Test A
    std::cout << "\n*** Test A - add, alter, print" << std::endl;
    {
        //Create and add values of various types
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
        //auto x = c["subnode1"]["sub3"];
        //assert(std::get<bool>(x.node)=="true"); //BUG returning 1 not true

        //add node using full IL
        c.add({{"sn1", { { "se1A", 1L }, { "se1B", false } }}});
        //add node using name and IL
        c.add("sn2", { { "se2A", "blah" }, { "se2B", false } });

        std::cout <<  "Json object to string: " << c << std::endl;

        //Next thing:  JSON array [][][]  (vector)
        //

        //Next: Serialise and Deserliase multi-nested levels - "Recursive descent parser" good approach
        //
    }   
}
