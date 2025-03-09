//
// Created by hy-20 on 2024/7/30.
//

#ifndef SLOWJSON_MERGER_HPP
#define SLOWJSON_MERGER_HPP

#include "concetps.hpp"

namespace slow_json {
    //此处只适用于inherit，不建议用户亲自调用，可能会带来一些意想不到的情况
    namespace helper{
        /**
         * 合并两个slow_json::static_dict，返回一个包含两个static_dict中所有健值对的新的static_dict对象
         * @tparam Ta 被合并的static_dict的类型
         * @tparam Tb 被合并的另一个static_dict的类型
         * @param a 被合并的static_dict对象
         * @param b 被合并的另一个static_dict对象
         * @return 合并之后的static_dict对象
         */
        template<slow_json::concepts::slow_json_static_dict Ta, slow_json::concepts::slow_json_static_dict Tb>
        static constexpr auto merge(const Ta &a, const Tb &b) {
            constexpr auto size1 = std::tuple_size_v<typename Ta::super_type>;
            constexpr auto size2 = std::tuple_size_v<typename Tb::super_type>;
            return [a, b]<std::size_t...index1, std::size_t...index2>(std::index_sequence<index1...> &&,
                                                                      std::index_sequence<index2...> &&) {
                return slow_json::static_dict{std::get<index1>(a)..., std::get<index2>(b)...};
            }(std::make_index_sequence<size1>{}, std::make_index_sequence<size2>{});
        }
        /**
         * 合并多个slow_json::static_dict，返回一个包含两所有static_dict中所有健值对的新的static_dict对象
         * @tparam Dict
         * @tparam Dicts
         */
        template<slow_json::concepts::slow_json_static_dict Dict,typename...Dicts>
        requires (slow_json::concepts::slow_json_static_dict<Dicts> && ...)
        static constexpr auto merge(const Dict&dict,const Dicts...dicts) {
            return merge(dict,merge(dicts...));
        }


        /**
         * 合并两个slow_json::polymorphic_dict对象，返回一个包含两个slow_json::polymorphic_dict中所有简直对的新的slow_json::polymorphic对象
         * @param d1 被合并的slow_json::polymorphic_dict对象
         * @param d2 被合并的另一个slow_json::polymorphic对象
         * @return 合并之后新的slow_json::polymorphic_dict对象
         */
        static slow_json::polymorphic_dict
        merge(const slow_json::polymorphic_dict &d1, const slow_json::polymorphic_dict &d2) {
            return {d1, d2};
        }
    }

    /**
     * 为派生类的序列化提供支持
     * @tparam SuperClass 父类类型
     * @tparam T 派生类类型
     * @param subclass_info 派生类的属性信息
     * @return 合并父类和派生类属性信息的slow_json::static_dict
     */
    template<typename SuperClass=void, concepts::slow_json_static_dict T>
    static constexpr auto inherit(const T &subclass_info) {
        if constexpr(std::is_void_v<SuperClass>){
            return subclass_info;
        }else {
            return slow_json::helper::merge(SuperClass::get_config(), subclass_info);
        }
    }

    /**
     * 为派生类的序列化提供支持
     * @tparam SuperClass 父类类型
     * @tparam T 派生类类型
     * @param subclass_info 派生类的属性信息
     * @return 合并父类和派生类属性信息的slow_json::polymorphic_dict
     */
    template<typename SuperClass, typename T>
    requires std::is_same_v<T, slow_json::polymorphic_dict>
    static constexpr auto inherit(const T &subclass_info) {
        return slow_json::helper::merge(SuperClass::get_config(), subclass_info);
    }
}
#endif //SLOWJSON_MERGER_HPP
