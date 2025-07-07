#include "concepts.hpp"
namespace slow_json{
    namespace details{
        struct dict;
    }
    class dynamic_dict;
    /**
     * 访问字典中的值并对符合类型的值调用回调函数
     * @tparam Args 可选的类型列表，如果未提供则使用默认类型列表 int, double, const char*
     * @tparam Dict 字典类型，必须是 slow_json::dict 或 slow_json::details::dynamic_dict
     * @tparam Fn 回调函数类型，必须可调用且参数类型匹配 Args 中的某一类型
     * @param dict 待访问的字典实例，使用完美转发以避免多余拷贝
     * @param fn 回调函数，当字典实际保存的类型与 Args 中某一类型匹配时会调用它
     */
    template<
            typename... Args,
            typename Dict,
            typename Fn
    >
    requires (
    concepts::is_contains_v<std::remove_const_t<std::remove_reference_t<Dict>>, slow_json::details::dict, slow_json::dynamic_dict> &&
            (std::is_invocable_v<Fn, Args> && ...)
    )
    void visit(Dict&& dict, Fn&& fn) {
        std::cout<<"type:"<< type_name_v<Dict> <<std::endl;
        // 如果没有显式提供任何类型参数，则使用默认类型列表
        if constexpr (sizeof...(Args) == 0) {
            // 递归调用自身，传入默认类型 int, double, const char*
            return visit<int64_t, double, std::string>(
                    std::forward<Dict>(dict),
                    std::forward<Fn>(fn)
            );
        } else {
            // 展开所有类型参数，对每个类型尝试从字典中提取并调用回调
            ([&]<typename T>() {
                using type = T;  // 当前正在检查的类型
                // as_type<type>() 判断字典是否保存该类型的值
                if (dict.template as_type<type>()) {
                    // cast<type>() 将字典中的值转换为目标类型并返回
                    fn(dict.template cast<type>());
                    return false;  // 利用and运算短路的特点，起到类似break的效果，跳过后续计算，优化性能
                }
                return true;
            }
                    .template operator()<Args>()
                    && ...);  // 短路运算优化，使用折叠表达式组合所有类型检查
        }
    }
}
