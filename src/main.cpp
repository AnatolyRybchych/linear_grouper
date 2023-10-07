#include <iostream>

#include <array>
#include <vector>

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

template <typename TraveralT>
concept TraversalType = requires(TraveralT trait){
    IntergralOrEnumType<typename TraveralT::Unit>;
    IntergralOrEnumType<typename TraveralT::Node>;

    std::same_as<decltype(TraveralT::unit_index), size_t(const typename TraveralT::Unit&)>;
    std::same_as<decltype(TraveralT::node_index), size_t(typename TraveralT::Node)>;

    std::unsigned_integral<decltype(TraveralT::NODES_COUNT)>;
    std::unsigned_integral<decltype(TraveralT::UNITS_COUNT)>;
    TraveralT::NODES_COUNT != 0;
    TraveralT::UNITS_COUNT != 0;
};

template <TraversalType TraversalT>
class Traversal{
    using Node = TraversalT::Node;
    using Unit = TraversalT::Unit;

    static constexpr size_t NODES_COUNT = TraversalT::NODES_COUNT;
    static constexpr size_t UNITS_COUNT = TraversalT::UNITS_COUNT;

    static inline size_t unit_index(const Unit &unit){ return TraversalT::unit_index(unit); }
    static inline size_t node_index(Node node){ return TraversalT::node_index(node); }
public:
    constexpr Traversal(){
        for(size_t node_idx = 0; node_idx != NODES_COUNT; node_idx++){
            for(size_t condition = 0; condition != UNITS_COUNT; condition++){
                nodes[node_idx][condition] = node_idx;
            }
        }
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

    template <typename ConditionChecker>
    constexpr void set_branch_if(Node from, Node to){
        ConditionChecker checker;

        size_t idx_from = node_index(from);
        size_t idx_to = node_index(to);

        for(size_t condition = 0; condition < UNITS_COUNT; condition ++){
            if(checker(condition)){
                _set_branch(idx_from, idx_to, condition);
            }
        }
    }

    template <typename Iter, typename Predicate>
    void traverse(Iter beg, Iter end, Node start_node, Predicate predicate){
        Iter chunk_beg = beg;

        Node cur_node = start_node;
        size_t cur_node_idx = node_index(cur_node);

        for(Iter it = beg; it != end; it++){
            Node node = nodes[cur_node_idx][unit_index(*it)];
            if(node != cur_node){
                if(chunk_beg != it){
                    if(!predicate(cur_node, chunk_beg, it)){
                        return;
                    }
                    chunk_beg = it;
                }

                cur_node = node;
                cur_node_idx = node_index(node);
            }
        }
    }
private:
    constexpr void _set_branch(size_t from, size_t to, size_t condition){
        nodes[from][condition] = to;
    }

    using NodeIndex = MinUint<NODES_COUNT-1>::Type;
    std::array<std::array<NodeIndex, UNITS_COUNT>, NODES_COUNT> nodes;
};


struct SplitTraversal{
    enum NodeT {
        SPACE,
        NOT_SPACE,

        __NODES_COUNT,
    };


    using Node = unsigned char;
    using Unit = unsigned char;
    static constexpr size_t UNITS_COUNT = 256;
    static constexpr size_t NODES_COUNT = __NODES_COUNT;
    
    static constexpr size_t unit_index(const Unit &unit){
        return unit;
    }

    static constexpr size_t node_index(Node node){
        return node;
    }
};

static constexpr Traversal<SplitTraversal> traversal(){
    Traversal<SplitTraversal> result;
    result.set_branch_if<decltype([](unsigned char ch){return std::isspace(ch);})>(SplitTraversal::NOT_SPACE, SplitTraversal::SPACE);
    result.set_branch_if<decltype([](unsigned char ch){return !std::isspace(ch);})>(SplitTraversal::SPACE, SplitTraversal::NOT_SPACE);
    return result;
}

int main(){
    std::string test = "some   text    here  ";

    traversal().traverse(test.cbegin(), test.cend(), SplitTraversal::NOT_SPACE, [] (auto node, std::string::const_iterator beg, std::string::const_iterator end) -> bool {
        std::cout << (size_t)node << ":" << std::string(beg, end) << std::endl;
        return true;
    });
    return 0;
}