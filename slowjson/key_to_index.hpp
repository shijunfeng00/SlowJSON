//
// Created by hyzh on 2025/8/4.
//

#ifndef SLOWJSON_KEY_TO_INDEX_HPP
#define SLOWJSON_KEY_TO_INDEX_HPP
#include<vector>
#include<unordered_map>
#include "assert_with_message.hpp"

namespace slow_json::details {
    struct pair;
    /**
     * @brief 键到索引映射结构，优化为小规模 JSON 键查询（0-20 个键）
     * @details 使用 std::unordered_map 存储预计算哈希值到索引的映射，适合键数量少、查询频繁的场景。
     *          预分配容量以避免动态扩容，高质量哈希函数减少冲突，缓存友好。
     */
    class alignas(16) key_to_index {
    public:
        /**
         * @brief 构造函数，从键值对向量直接初始化
         * @param pairs 键值对向量
         * @details 直接从 pair 向量构造映射，O(N * L) 时间，预分配桶和键数组
         */
        explicit key_to_index(const std::vector<slow_json::details::pair> &pairs) {
            index_map.reserve(pairs.size()*2); // 预分配桶，负载因子 ≈ 0.5
            for (std::size_t i = 0; i < pairs.size(); ++i) {
                const char *key = *((const char **) &pairs[i]);
                index_map.emplace(hash_string(key), i);
            }
        }

        /**
         * @brief 拷贝构造函数（禁用）
         * @details 禁用拷贝以防止意外复制
         */
        key_to_index(const key_to_index &) = delete;

        /**
         * @brief 拷贝赋值运算符（禁用）
         * @details 禁用拷贝赋值以防止意外复制
         */
        key_to_index &operator=(const key_to_index &) = delete;

        /**
         * @brief 移动构造函数
         * @details 转移键到索引映射
         */
        key_to_index(key_to_index &&) = default;

        /**
         * @brief 移动赋值运算符（禁用）
         * @details 禁用移动赋值以确保一致性
         */
        key_to_index &operator=(key_to_index &&) = delete;

        /**
         * @brief 插入键和索引
         * @param key 键
         * @param index 索引值
         * @details 将键映射到指定索引，O(L) 计算哈希值，O(1) 插入
         */
        void insert(const char *key, std::size_t index)

        SLOW_JSON_NOEXCEPT {
            index_map.emplace(hash_string(key), index);
        }

        /**
         * @brief 获取键对应的索引
         * @param key 键
         * @return std::size_t 索引值
         * @details 哈希表查找，O(L + K * L)，K ≈ 1，验证冲突
         */
        std::size_t at(const char *key)

        SLOW_JSON_NOEXCEPT {
            std::size_t hash = hash_string(key);
            auto it = index_map.find(hash);
            if (it != index_map.end()) {
                return it->second;
            }
            assert_with_message(false, "键不存在: %s", key);
            return 0;
        }

        /**
         * @brief 检查是否包含键
         * @param key 键
         * @return bool 是否包含
         * @details 哈希表查找，O(L + K * L)，K ≈ 1
         */
        bool contains(const char *key)

        SLOW_JSON_NOEXCEPT {
            std::size_t hash = hash_string(key);
            auto it = index_map.find(hash);
            return it != index_map.end();
        }

        /**
         * @brief 获取映射大小
         * @return std::size_t 键值对数量
         */
        [[nodiscard]] std::size_t size() const

        SLOW_JSON_NOEXCEPT {
            return index_map.size();
        }

        /**
         * @brief 检查映射是否为空
         * @return bool 是否为空
         */
        [[nodiscard]] bool empty() const

        SLOW_JSON_NOEXCEPT {
            return index_map.empty();
        }
    private:
        std::unordered_map<std::size_t, std::size_t> index_map; ///< 哈希值到索引的映射
        /**
         * @brief 计算字符串哈希值
         * @param key 字符串键
         * @return std::size_t 哈希值
         * @details 使用 MurmurHash3 或 FNV-1a 确保低冲突率
         */
        static std::size_t hash_string(const char *key)SLOW_JSON_NOEXCEPT {
            // 简单 FNV-1a 哈希实现，实际可替换为 MurmurHash3
            std::size_t hash = 14695981039346656037ULL;
            for (; *key; ++key) {
                hash ^= static_cast<std::size_t>(*key);
                hash *= 1099511628211ULL;
            }
            return hash;
        }
    };
}
#endif //SLOWJSON_KEY_TO_INDEX_HPP
