#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

template <typename IntergralOrEnumT>
concept IntergralOrEnumType = std::unsigned_integral<IntergralOrEnumT>
    || std::is_enum<IntergralOrEnumT>::value;

template<size_t SIZE>
struct MinUint {
    typedef 
    typename std::conditional<(SIZE <= UINT8_MAX), 
        uint8_t,
        typename std::conditional<(SIZE <= UINT16_MAX), 
            uint16_t,
            typename std::conditional<(SIZE <= UINT32_MAX), 
                uint32_t,
                uint64_t
            >::type
        >::type
    >::type Type;
};

template <typename LinearGrouperT>
concept LinearGrouperType = requires(LinearGrouperT trait){
    IntergralOrEnumType<typename LinearGrouperT::Unit>;
    IntergralOrEnumType<typename LinearGrouperT::Node>;

    std::same_as<decltype(LinearGrouperT::unit_index), size_t(const typename LinearGrouperT::Unit&)>;
    std::same_as<decltype(LinearGrouperT::node_index), size_t(typename LinearGrouperT::Node)>;

    std::unsigned_integral<decltype(LinearGrouperT::NODES_COUNT)>;
    std::unsigned_integral<decltype(LinearGrouperT::UNITS_COUNT)>;
    LinearGrouperT::NODES_COUNT != 0;
    LinearGrouperT::UNITS_COUNT != 0;
};

template <LinearGrouperType LinearGrouperT>
class LinearGrouper{
    using Node = LinearGrouperT::Node;
    using Unit = LinearGrouperT::Unit;

    static constexpr size_t NODES_COUNT = LinearGrouperT::NODES_COUNT;
    static constexpr size_t UNITS_COUNT = LinearGrouperT::UNITS_COUNT;

    static inline size_t unit_index(const Unit &unit){ return LinearGrouperT::unit_index(unit); }
    static inline size_t node_index(Node node){ return LinearGrouperT::node_index(node); }

    template <typename T, typename U>
    static auto translate_node_exists(U *from) -> decltype(T::translate_node(*from), std::true_type{});

    template <typename T, typename U>
    static std::false_type translate_node_exists(...);

    template <typename T, typename U>
    static inline std::enable_if<decltype(translate_node_exists<T, U>(nullptr))::value, U>::type 
    translate_node(Node node) {
        return LinearGrouperT::translate_node(node);
    }

    template <typename T, typename U>
    static inline std::enable_if<!decltype(translate_node_exists<T, U>(nullptr))::value, U>::type 
    translate_node(Node node) {
        return node;
    }

public:
    constexpr LinearGrouper(){
        for(size_t node_idx = 0; node_idx != NODES_COUNT; node_idx++){
            for(size_t condition = 0; condition != UNITS_COUNT; condition++){
                nodes[node_idx][condition] = node_idx;
            }
        }
    }

    constexpr void set_branch_i(Node from, Node to, size_t &condition_index){
        _set_branch(node_index(from), node_index(to), condition_index);
    }

    constexpr void set_branch(Node from, Node to, const Unit &condition){
        _set_branch(node_index(from), node_index(to), unit_index(condition));
    }

    constexpr void set_all_branches(Node from, Node to){
        size_t idx_from = node_index(from);
        size_t idx_to = node_index(to);
        for(size_t condition = 0; condition < UNITS_COUNT; condition++){
            _set_branch(idx_from, idx_to, condition);
        }
    }

    template <typename Filter>
    void set_branch_if(Node from, Node to, Filter filter)
        requires std::invocable<Filter, size_t> && std::predicate<Filter, size_t>
    {
        size_t idx_from = node_index(from);
        size_t idx_to = node_index(to);

        for(size_t condition = 0; condition < UNITS_COUNT; condition ++){
            if(filter(condition)){
                _set_branch(idx_from, idx_to, condition);
            }
        }
    }
    

    template <typename Iter, typename Predicate>
    void traverse(Iter beg, Iter end, Node start_node, Predicate predicate) 
        requires std::forward_iterator<Iter> && std::invocable<Predicate, Node, Iter, Iter>
    {
        Iter chunk_beg = beg;

        Node cur_node = start_node;
        Node cur_node_orig = translate_node<LinearGrouperT, Node>(cur_node);
        size_t cur_node_idx = node_index(cur_node);

        for(Iter it = beg; it != end; std::advance(it, 1)){
            Node node = nodes[cur_node_idx][unit_index(*it)];
            Node node_orig = translate_node<LinearGrouperT, Node>(node);
            if(node != cur_node){
                if(cur_node_orig != node_orig &&  chunk_beg != it){
                    if(!predicate(cur_node, chunk_beg, it)){
                        return;
                    }
                    chunk_beg = it;
                }

                cur_node = node;
                cur_node_orig = node_orig;
                cur_node_idx = node_index(node);
            }
        }

        if(chunk_beg != end){
            predicate(cur_node, chunk_beg, end);
        }
    }
private:
    constexpr void _set_branch(size_t from, size_t to, size_t condition){
        nodes[from][condition] = to;
    }

    using NodeIndex = MinUint<NODES_COUNT-1>::Type;
    std::array<std::array<NodeIndex, UNITS_COUNT>, NODES_COUNT> nodes;
};
