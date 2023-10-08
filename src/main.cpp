#include <iostream>

#include <linear_grouper.hpp>

struct TestGrouper{
    enum _Node {
        UNDEFINED,
        SPACE,
        SYMBOL,
        SYMBOL_START,
        NUMBER,

        __NODES_COUNT,
    };

    using Unit = unsigned char;
    using Node = unsigned char;
    static constexpr size_t UNITS_COUNT = 256;
    static constexpr size_t NODES_COUNT = __NODES_COUNT;
    
    static constexpr size_t unit_index(const Unit &unit){
        return unit;
    }

    static constexpr size_t node_index(Node node){
        return node;
    }

    static Node translate_node(Node node){
        static std::array<unsigned char, __NODES_COUNT> nodes {
            UNDEFINED,
            SPACE,
            SYMBOL,
            SYMBOL,
            NUMBER,
        };

        return nodes[node];
    }

    static constexpr const char * node_string(Node node){
        #define __NODE_STR_CASE(NODE) if (node == NODE) return #NODE

        __NODE_STR_CASE(UNDEFINED);
        __NODE_STR_CASE(SPACE);
        __NODE_STR_CASE(SYMBOL_START);
        __NODE_STR_CASE(SYMBOL);
        __NODE_STR_CASE(NUMBER);

        return "UNKNOWN";
    };
};

static LinearGrouper<TestGrouper> traversal = []() {
    LinearGrouper<TestGrouper> result;
    result.set_all_branches(TestGrouper::SPACE, TestGrouper::UNDEFINED);
    result.set_all_branches(TestGrouper::SYMBOL_START, TestGrouper::UNDEFINED);
    result.set_all_branches(TestGrouper::SYMBOL, TestGrouper::UNDEFINED);
    result.set_all_branches(TestGrouper::NUMBER, TestGrouper::UNDEFINED);
    result.set_all_branches(TestGrouper::UNDEFINED, TestGrouper::UNDEFINED);

    result.set_branch_if(TestGrouper::SPACE, TestGrouper::SPACE, [](auto node){return std::isspace(node);});
    result.set_branch_if(TestGrouper::SYMBOL_START, TestGrouper::SPACE, [](auto node){return std::isspace(node);});
    result.set_branch_if(TestGrouper::SYMBOL, TestGrouper::SPACE, [](auto node){return std::isspace(node);});
    result.set_branch_if(TestGrouper::NUMBER, TestGrouper::SPACE, [](auto node){return std::isspace(node);});
    result.set_branch_if(TestGrouper::UNDEFINED, TestGrouper::SPACE, [](auto node){return std::isspace(node);});

    result.set_branch_if(TestGrouper::SPACE, TestGrouper::NUMBER, [](auto node){return std::isdigit(node);});
    result.set_branch_if(TestGrouper::UNDEFINED, TestGrouper::NUMBER, [](auto node){return std::isdigit(node);});
    result.set_branch_if(TestGrouper::NUMBER, TestGrouper::NUMBER, [](auto node){return std::isdigit(node);});

    result.set_branch_if(TestGrouper::UNDEFINED, TestGrouper::SYMBOL_START, [](auto node){return std::isalpha(node) || node == '_';});
    result.set_branch_if(TestGrouper::SPACE, TestGrouper::SYMBOL_START, [](auto node){return std::isalpha(node) || node == '_';});

    result.set_branch_if(TestGrouper::SYMBOL_START, TestGrouper::SYMBOL, [](auto node){return std::isalnum(node) || node == '_';});
    result.set_branch_if(TestGrouper::SYMBOL, TestGrouper::SYMBOL, [](auto node){return std::isalnum(node) || node == '_';});

    return result;
}();

int main(){
    std::string test = "123 is number and @$#@ is undefined";

    traversal.traverse(test.cbegin(), test.cend(), TestGrouper::UNDEFINED, [] (auto node, std::string::const_iterator beg, std::string::const_iterator end) -> bool {
        std::cout << TestGrouper::node_string(node) << ":" << std::string(beg, end) << std::endl;
        return true;
    });
    return 0;
}