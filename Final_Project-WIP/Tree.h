/*
    CPSC 5700: Computer Graphics
    Ana Carolina de Souza Mendes

    Tree.h: generates points for 3D tree sketch
*/

#pragma once

#include <glad.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <queue>
#include "GLXtras.h"

class Tree {
public:
    Tree();
    void generateTree(std::vector<vec3> &points);
private:
    std::vector<char> rulesFrom;
    std::vector<std::queue<char>> rulesTo;
    double forwardStep;
    double xyTurn;
    double yzTurn;
    double startxyAngle;
    double startyzAngle;
    int numGenerations;
    std::queue<char> renderQueue;
    void populateRulesVectors(std::string rule1, std::string rule2);
    void populateRenderQueue();
};