//
// Created by hy-20 on 2024/7/30.
//

#ifndef SLOWJSON_MERGER_HPP
#define SLOWJSON_MERGER_HPP

#include "concepts.hpp"

namespace slow_json {
    //下面的merge函数，主要用于inherit，按照设计并不是主要接口，不建议使用者调用，可能会带来一些意想不到的情况
    namespace details{

        /**
         * 合并两个slow_json::dict，返回一个包含两个slow_json::dict中所有健值对的新的slow_json::dict对象
         * @param a 被合并的slow_json::dict对象
         * @param b 被合并的另一个slow_json::dict对象
         * @return 合并之后的slow_json::dict对象
         */
        inline slow_json::dict merge(slow_json::dict&&a,slow_json::dict&&b){
            slow_json::dict c{};
            auto&a_data=a._data;
            auto&b_data=b._data;
            auto&c_data=c._data;
            c_data.reserve(a_data.size()+b_data.size());
            for(auto&pair:a_data){
                c_data.emplace_back(std::move(pair));
            }
            for(auto&pair:b_data){
                c_data.emplace_back(std::move(pair));
            }
            a_data.clear();
            b_data.clear();
            return c;
        }

        /**
         * 合并多个slow_json::dict，返回一个包含两个slow_json::dict中所有健值对的新的slow_json::dict对象
         * @tparam Dicts 多个slow_json::dict组成的变参
         * @param slow_json::dict对象变参
         */
        template<typename...Dicts>
        requires (std::is_same_v<Dicts,slow_json::dict> && ...)
        inline slow_json::dict merge(slow_json::dict&&dict,Dicts&&...dicts){
            auto result=merge(std::move(dicts)...);
            return merge(std::move(dict),std::move(result));
        }

        /**
         * 合并两个slow_json::static_dict，返回一个包含两个static_dict中所有健值对的新的static_dict对象
         * @tparam Ta 被合并的static_dict的类型
         * @tparam Tb 被合并的另一个static_dict的类型
         * @param a 被合并的static_dict对象
         * @param b 被合并的另一个static_dict对象
         * @return 合并之后的static_dict对象
         */
        template<slow_json::concepts::slow_json_static_dict Ta, slow_json::concepts::slow_json_static_dict Tb>
        inline constexpr auto merge(const Ta &a, const Tb &b) {
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
        inline constexpr auto merge(const Dict&dict,const Dicts...dicts) {
            return merge(dict,merge(dicts...));
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
    inline constexpr auto inherit(const T &subclass_info) {
        if constexpr(std::is_void_v<SuperClass>){
            return subclass_info;
        }else {
            return slow_json::details::merge(SuperClass::get_config(), subclass_info);
        }
    }

    /**
     * 为派生类的序列化提供支持
     * @tparam SuperClass 父类类型
     * @tparam T 派生类类型
     * @param subclass_info 派生类的属性信息
     * @return 合并父类和派生类属性信息的slow_json::dict
     */
    template<typename SuperClass, typename T>
    requires std::is_same_v<T, slow_json::dict>
    inline constexpr auto inherit(T&&subclass_info) {
        return slow_json::details::merge(SuperClass::get_config(), std::move(subclass_info));
    }
}
#endif //SLOWJSON_MERGER_HPP
