#include "concepts.hpp"
namespace slow_json{
    namespace details{
        struct dynamic_dict;
        struct dict;
    }
    template<typename... Args,typename Dict,typename Fn>
    requires (concepts::is_contains_v<Dict,slow_json::dict,slow_json::dynamic_dict> && (std::is_invocable_v<Fn,Args> && ...))
    void visit(Dict&& dict, Fn&& fn) {
        if constexpr(sizeof...(Args)==0){
            return visit<int,double,const char*>(std::forward<Dict>(dict),std::forward<Fn>(fn));
        }else{
            // 获取原始对象的引用（避免拷贝）
            // 直接展开类型包（不使用索引序列）
            ([&]<typename T>() {
                using type = T;
                if (dict.template as_type<type>()) {
                    fn(dict.template cast<type>());
                    return false;
                }
                return true;
            }.template operator()<Args>() && ...);
        }
    }
}
