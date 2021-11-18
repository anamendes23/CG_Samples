/*
	CPSC 5700: Computer Graphics
	Ana Carolina de Souza Mendes

	Tree.cpp: generates points for 3D tree sketch
*/

#include "Tree.h"
#include <iostream>
#include <cmath>
#include <stack>

#define M_PI 3.14159265358979323846

Tree::Tree() {
	this->forwardStep = 0.007;
	this->xyTurn = 25 * M_PI / 180.0;
	this->yzTurn = 45 * M_PI / 180.0;
	this->startxyAngle = M_PI / 2.0;
	this->startyzAngle = M_PI / 2.0;

	this->numGenerations = 7;

    populateRulesVectors("F[+X][-X]FX", "FF");
    populateRenderQueue();
}

void Tree::generateTree(std::vector<vec3>& points) {
    char renderCommand;
    vec3 currPos = vec3(0, -1, 0);
    double theta = 0.0;
    double r = 0.0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    std::stack<vec3> posStack;
    std::stack<double> anglexyStack;
    std::stack<double> angleyzStack;

    while (!renderQueue.empty()) {

        renderCommand = renderQueue.front();
        renderQueue.pop();
        //printf("%c", renderCommand);
        switch (renderCommand) {
        case 'F':
            points.push_back(currPos);

            theta = atan2(currPos.y, currPos.x);
            x = forwardStep * cos(theta);
            y = forwardStep * sin(theta);

            theta = atan2(currPos.y, currPos.z);
            z = forwardStep * cos(theta);
            //printf("theta: %f | x: %f| y: %f\n", theta, x, y);
            
            r = sqrt(x * x + y * y);
            x = r * cos(startxyAngle);
            y = r * sin(startxyAngle);
            
            r = sqrt(z * z + y * y);
            z = r * cos(startyzAngle);
            //printf("sAngle: %f | r: %f | x: %f | y: %f\n", startAngle, r, x, y);

            currPos.x = currPos.x + x;
            currPos.y = currPos.y + y;
            currPos.z = currPos.z + z;
            points.push_back(currPos);

            //printf("currx: %f | curry: %f\n", currPos.x, currPos.y);
            break;

        case '-':
            startxyAngle += xyTurn;
            startyzAngle += yzTurn;
            break;

        case '+':
            startxyAngle -= xyTurn;
            startyzAngle -= yzTurn;
            break;
        case '[':
            posStack.push(currPos);
            anglexyStack.push(startxyAngle);
            angleyzStack.push(startyzAngle);
            break;
        case ']':
            currPos = posStack.top();
            posStack.pop();
            startxyAngle = anglexyStack.top();
            anglexyStack.pop();
            startyzAngle = angleyzStack.top();
            angleyzStack.pop();
            break;
        case 'X':
            break;
        default:
            break;
        }
    }
}

void Tree::populateRulesVectors(std::string rule1, std::string rule2) {
    // rules from
    this->rulesFrom.push_back('X');
    this->rulesFrom.push_back('F');
    // rules to
    std::queue<char> queue1;
    for (int i = 0; i < rule1.length(); i++) {
        queue1.push(rule1[i]);
    }
    this->rulesTo.push_back(queue1);

    std::queue<char> queue2;
    for (int i = 0; i < rule2.length(); i++) {
        queue2.push(rule2[i]);
    }
    this->rulesTo.push_back(queue2);
}

void Tree::populateRenderQueue() {
    std::queue<char> output;
    output.push('X');

    std::queue<char> input;

    for (int gen = 0; gen < numGenerations; gen++)
    {
        input = output; // last generation's output is this one's input
        std::queue<char> temp;
        output = temp;
        while (!input.empty())
        {
            char nextInput = input.front();
            input.pop();
            // look through the from-list to find a rule to invoke
            bool ruleFound = false;
            for (int i = 0; !ruleFound && i < rulesFrom.size(); i++)
                if (rulesFrom[i] == nextInput)
                {
                    ruleFound = true;
                    // append a copy of this rule's right side onto the 
                    // end of the output queue
                    std::queue<char> outTemp = rulesTo[i];
                    char currChar;
                    for (int i = 0; i = outTemp.size(); i++) {
                        currChar = outTemp.front();
                        outTemp.pop();
                        output.push(currChar);
                    }
                }
            // if no rule was found, then just copy this command to output
            if (!ruleFound)
                output.push(nextInput);
        }
    }
    renderQueue = output;
}