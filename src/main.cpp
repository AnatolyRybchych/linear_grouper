#include <iostream>

#include <array>
#include <vector>

template <typename IntergralOrEnumT>
concept IntergralOrEnumType = std::integral<IntergralOrEnumT>
    || std::is_enum<IntergralOrEnumT>::value;

template <typename UnitT>
concept UnitType = requires(){
    std::same_as<decltype(UnitT::COUNT), size_t>;
    UnitT::COUNT != 0;

    IntergralOrEnumType<typename UnitT::Type>;

    std::same_as<decltype(UnitT::get), size_t (typename UnitT::Type)>;
};

template <typename NodeT>
concept NodeType = requires(NodeT unit){
    std::same_as<decltype(NodeT::COUNT), size_t>;
    NodeT::COUNT != 0;

    
};

template <typename TraveralT>
concept TraversalType = requires(TraveralT trait){
    IntergralOrEnumType<typename TraveralT::Unit>;
    IntergralOrEnumType<typename TraveralT::Node>;

    std::same_as<decltype(TraveralT::unit_index), size_t(typename TraveralT::Unit)>;
    std::same_as<decltype(TraveralT::node_index), size_t(typename TraveralT::Node)>;
    std::same_as<decltype(TraveralT::index_unit), typename TraveralT::Unit(size_t)>;
    std::same_as<decltype(TraveralT::index_node), typename TraveralT::Node(size_t)>;

    std::same_as<decltype(TraveralT::NODES_COUNT), size_t>;
    std::same_as<decltype(TraveralT::UNITS_COUNT), size_t>;
    TraveralT::NODES_COUNT != 0;
    TraveralT::UNITS_COUNT != 0;
};

template <TraversalType TraversalT>
class Traversal{
    using Node = TraversalT::Node;
    using Unit = TraversalT::Unit;

    static constexpr size_t NODES_COUNT = TraversalT::NODES_COUNT;
    static constexpr size_t UNITS_COUNT = TraversalT::UNITS_COUNT;

    static inline size_t unit_index(Unit unit){ return TraversalT::unit_index(unit); }
    static inline Unit index_unit(size_t index){ return TraversalT::index_unit(index); }
    static inline size_t node_index(Node node){ return TraversalT::node_index(node); }
    static inline Node index_node(size_t index){ return TraversalT::index_node(index); }

public:
    constexpr Traversal(){
        for(size_t node_idx = 0; node_idx < NODES_COUNT; node_idx++){
            Node node = index_node(node_idx);
            set_all_branches(node, node);
        }
    }

    constexpr void set_branch(Node from, Node to, Unit condition){
        nodes[node_index(from)][unit_index(condition)] = to;
    }

    constexpr void set_all_branches(Node from, Node to){
        for(size_t condition = 0; condition < UNITS_COUNT; condition++){
            set_branch(from, to, index_unit(condition));
        }
    }

    template <typename ConditionChecker>
    constexpr void set_branch_if(Node from, Node to){
        ConditionChecker checker;

        for(size_t condition = 0; condition < UNITS_COUNT; condition ++){
            if(checker(condition)){
                set_branch(from, to, index_unit(condition));
            }
        }
    }

    template <typename Iter, typename Predicate>
    void traverse(Iter beg, Iter end, Node start_node, Predicate predicate){
        Iter chunk_beg = beg;
        Node cur_node = start_node;
        for(Iter it = beg; it != end; it++){
            Node node = nodes[node_index(cur_node)][unit_index(*it)];
            if(node != cur_node){
                if(chunk_beg != it){
                    if(!predicate(cur_node, chunk_beg, it)){
                        return;
                    }
                    chunk_beg = it;
                }
                cur_node = node;
            }
        }
    }
private:
    std::array<std::array<Node, UNITS_COUNT>, NODES_COUNT> nodes;
};


struct SplitTraversal{
    enum Node {
        SPACE,
        NOT_SPACE,

        NODES_COUNT,
    };

    using Unit = unsigned char;
    static constexpr size_t UNITS_COUNT = 256;
    
    static constexpr size_t unit_index(Unit unit){
        return unit;
    }

    static constexpr size_t node_index(Node node){
        return node;
    }

    static constexpr Unit index_unit(size_t index){
        return index;
    }

    static constexpr Node index_node(size_t index){
        return (Node)index;
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