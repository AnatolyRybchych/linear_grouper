#include <iostream>

#include <array>
#include <vector>

enum class Node{
    SPACE,
    NOT_SPACE,

    _COUNT,
};


class Traversal{
public:
    constexpr Traversal(){
        for(size_t node = 0; node < nodes.size(); node++){
            set_all_branches((Node)node, (Node)node);
        }
    }

    constexpr void set_branch(Node from, Node to, unsigned char condition){
        nodes[(size_t)from][condition] = to;
    }

    constexpr void set_all_branches(Node from, Node to){
        for(size_t condition = 0; condition < 256; condition ++){
            set_branch(from, to, condition);
        }
    }

    template <typename ConditionChecker>
    constexpr void set_branch_if(Node from, Node to){
        ConditionChecker checker;

        for(size_t condition = 0; condition < 256; condition ++){
            if(checker(condition)){
                set_branch(from, to, condition);
            }
        }
    }

    template <typename Iter, typename Predicate>
    void traverse(Iter beg, Iter end, Node start_node, Predicate predicate){
        Iter chunk_beg = beg;
        Node cur_node = start_node;
        for(Iter it = beg; it != end; it++){
            Node node = nodes[(size_t)cur_node][*it];
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
    std::array<std::array<Node, 256>, (size_t)Node::_COUNT> nodes;
};

static constexpr Traversal traversal(){
    Traversal result;
    result.set_branch_if<decltype([](unsigned char ch){return std::isspace(ch);})>(Node::NOT_SPACE, Node::SPACE);
    result.set_branch_if<decltype([](unsigned char ch){return !std::isspace(ch);})>(Node::SPACE, Node::NOT_SPACE);
    return result;
}

int main(){
    std::string test = "some   text    here  ";

    traversal().traverse(test.cbegin(), test.cend(), Node::NOT_SPACE, [] (Node node, std::string::const_iterator beg, std::string::const_iterator end) -> bool {
        std::cout << (size_t)node << ":" << std::string(beg, end) << std::endl;
        return true;
    });
    return 0;
}