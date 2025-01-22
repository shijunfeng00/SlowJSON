//
// Created by hyzh on 2025/1/22.
//

#ifndef SLOWJSON_OSTREAM_HPP
#define SLOWJSON_OSTREAM_HPP
#include <iostream>
#include "concetps.hpp"
#include "supported.hpp"
#include "dump.hpp"
namespace slow_json::ostream {
    class ostream {
    public:
        template<slow_json::concepts::dump_supported JSONable>
        friend ostream& operator<<(ostream&cout,const JSONable&object);
        void set_indent(std::optional<int>indent){
            this->_indent=indent;
        }
    private:
        slow_json::Buffer _buffer;
        std::optional<int>_indent;
    }cout;

    ostream&operator<<(ostream&cout,std::ostream& (*pf)(std::ostream&)) {
        std::cout<<pf;
        return cout;
    }
    template<slow_json::concepts::dump_supported JSONable>
    ostream& operator<<(ostream&cout,const JSONable&object){
        if constexpr(slow_json::concepts::string<JSONable>){
            std::cout<<object;
        }else if constexpr (slow_json::concepts::enumerate<JSONable>){
            std::cout<<slow_json::enum2string(object);
        }
        else {
            slow_json::dumps(cout._buffer, object, cout._indent);
            std::cout << cout._buffer;
            cout._buffer.clear();
        }
        return cout;
    }
}

#endif //SLOWJSON_OSTREAM_HPP
