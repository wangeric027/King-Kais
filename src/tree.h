#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Tree
{
public:
    Tree();

    static std::string createTree();
    static std::vector<glm::mat4> createCTMList(std::string treeString, glm::vec3 posList);
};

#endif // TREE_H
