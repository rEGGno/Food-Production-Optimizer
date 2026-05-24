#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <limits>
#include <algorithm>

using namespace std;

// Node Category Tracking Enum
enum NodeType { INGREDIENT_CHOICE, COMPOSITE_PRODUCT };

// Structure to represent a recipe option/pathway
struct Edge {
    int targetNode;
    double cost;
    string actionDescription;
};

// Global structures to map string names to integer IDs for the graph
unordered_map<string, int> nodeIDs;
unordered_map<int, string> nodeNames;
unordered_map<int, NodeType> nodeTypes; // Explicitly stores if a node is a choice or a combination
int nodeCounter = 0;

// Function to safely get or create a node ID
int getNodeID(const string& name, NodeType type) {
    if (nodeIDs.find(name) == nodeIDs.end()) {
        nodeIDs[name] = nodeCounter;
        nodeNames[nodeCounter] = name;
        nodeTypes[nodeCounter] = type;
        nodeCounter++;
    }
    return nodeIDs[name];
}

// Computes the absolute minimum cost to obtain a node
double computeMinCosts(int u, 
                       const unordered_map<int, vector<Edge>>& graph, 
                       unordered_map<int, double>& memoTable, 
                       unordered_map<int, int>& bestChoiceTracker) {
    
    if (memoTable.find(u) != memoTable.end()) {
        return memoTable[u];
    }

    // Leaf nodes (terminal raw materials)
    if (graph.find(u) == graph.end() || graph.at(u).empty()) {
        return memoTable[u] = 0;
    }

    // Use the explicit node type configuration map rather than unstable string checking
    if (nodeTypes[u] == INGREDIENT_CHOICE) {
        double minCost = numeric_limits<double>::infinity();
        int bestEdgeIndex = -1;

        // Evaluate all user-input options to choose ONLY the absolute cheapest single choice
        for (int i = 0; i < graph.at(u).size(); ++i) {
            const auto& edge = graph.at(u)[i];
            double choiceCost = edge.cost + computeMinCosts(edge.targetNode, graph, memoTable, bestChoiceTracker);
            if (choiceCost < minCost) {
                minCost = choiceCost;
                bestEdgeIndex = i;
            }
        }
        bestChoiceTracker[u] = bestEdgeIndex; // Save the winning choice option index
        return memoTable[u] = minCost;
    } else {
        // If it's a composite product, aggregate ALL of its structural ingredients
        double structuralTotal = 0;
        for (const auto& edge : graph.at(u)) {
            structuralTotal += edge.cost + computeMinCosts(edge.targetNode, graph, memoTable, bestChoiceTracker);
        }
        return memoTable[u] = structuralTotal;
    }
}

// Strict Top-Down Trace to construct the optimized printout receipt
void generateReceipt(int u, 
                     const unordered_map<int, vector<Edge>>& graph, 
                     const unordered_map<int, int>& bestChoiceTracker, 
                     vector<string>& receiptLines, 
                     double& totalCost) {
    
    if (graph.find(u) == graph.end() || graph.at(u).empty()) {
        return;
    }

    if (nodeTypes[u] == INGREDIENT_CHOICE) {
        if (bestChoiceTracker.find(u) != bestChoiceTracker.end()) {
            int optimalIndex = bestChoiceTracker.at(u);
            const auto& edge = graph.at(u)[optimalIndex];
            
            if (edge.cost > 0) {
                string line = "- " + edge.actionDescription + " : Rp" + to_string((int)edge.cost);
                receiptLines.push_back(line);
                totalCost += edge.cost;
            }
            generateReceipt(edge.targetNode, graph, bestChoiceTracker, receiptLines, totalCost);
        }
    } else {
        // Process ALL required structural ingredients for composite recipes
        for (const auto& edge : graph.at(u)) {
            if (edge.cost > 0) {
                string line = "- " + edge.actionDescription + " : Rp" + to_string((int)edge.cost);
                receiptLines.push_back(line);
                totalCost += edge.cost;
            }
            generateReceipt(edge.targetNode, graph, bestChoiceTracker, receiptLines, totalCost);
        }
    }
}

int main() {
    unordered_map<int, vector<Edge>> graph;
    vector<int> ingredientCategoryNodes;
    string bufferInput;

    cout << "==================================================\n";
    cout << "       DYNAMIC SUPPLY PATHWAY CONFIGURATOR        \n";
    cout << "==================================================\n";

    // 1. Dynamic Setup Wizard Loop using explicit type parameters
    cout << "How many core ingredients do you want to manage? ";
    getline(cin, bufferInput);
    int totalIngredients = stoi(bufferInput);

    for (int i = 0; i < totalIngredients; ++i) {
        cout << "\nEnter name for Ingredient #" << (i + 1) << " (e.g., 1 kg Flour): ";
        string ingredientName;
        getline(cin, ingredientName);
        
        // Explicitly declare this node as an ingredient choice intersection
        int ingredientNodeID = getNodeID(ingredientName, INGREDIENT_CHOICE);
        ingredientCategoryNodes.push_back(ingredientNodeID);

        cout << "How many sourcing options/pathways exist for " << ingredientName << "? ";
        getline(cin, bufferInput);
        int totalOptions = stoi(bufferInput);

        for (int j = 0; j < totalOptions; ++j) {
            cout << "  Option " << (j + 1) << " description (e.g., Buy from Market A): ";
            string description;
            getline(cin, description);

            cout << "  Price/Cost for this option (Rp): ";
            getline(cin, bufferInput);
            double cost = stod(bufferInput);

            // Leaf nodes are terminal endpoints
            string childNodeName = ingredientName + " via Choice " + to_string(j + 1);
            int childNodeID = getNodeID(childNodeName, COMPOSITE_PRODUCT);

            graph[ingredientNodeID].push_back({childNodeID, cost, description});
        }
    }

    // 2. Final Product Assignment
    cout << "\n==================================================\n";
    cout << "Create a final composite product name (e.g., Custom Bread)\n";
    cout << "This item will combine all ingredients listed above.\n";
    cout << "Product name: ";
    string customProductName;
    getline(cin, customProductName);
    
    // Explicitly declare the final target item as a combination product
    int productNodeID = getNodeID(customProductName, COMPOSITE_PRODUCT);

    for (int ingID : ingredientCategoryNodes) {
        graph[productNodeID].push_back({ingID, 0, "Include required component: " + nodeNames[ingID]});
    }

    for (int i = 0; i < nodeCounter; ++i) {
        if (graph.find(i) == graph.end()) {
            graph[i] = {};
        }
    }

    // 3. Operational User Interface
    cout << "\n==================================================\n";
    cout << "Available target optimization items:\n";
    cout << " - " << customProductName << " (Combines all ingredients)\n";
    for (int ingID : ingredientCategoryNodes) {
        cout << " - " << nodeNames[ingID] << " (Optimize just this item alone)\n";
    }
    cout << "--------------------------------------------------\n";

    cout << "Enter the item you want to optimize: ";
    string targetProduct;
    getline(cin, targetProduct);

    if (nodeIDs.find(targetProduct) == nodeIDs.end()) {
        cout << "\nError: Selection not found in database.\n";
        return 1;
    }

    cout << "Enter your available budget (Rp): ";
    getline(cin, bufferInput);
    double budget = stod(bufferInput);

    // 4. Run Core Shortest-Path Graph Engines
    int startNode = nodeIDs[targetProduct];
    unordered_map<int, double> memoTable;
    unordered_map<int, int> bestChoiceTracker;

    computeMinCosts(startNode, graph, memoTable, bestChoiceTracker);

    double totalCost = 0;
    vector<string> receiptLines;
    generateReceipt(startNode, graph, bestChoiceTracker, receiptLines, totalCost);

    // 5. Output Final Single-Path Receipt
    cout << "\n==================================================\n";
    cout << "                OPTIMAL RECEIPT                   \n";
    cout << "==================================================\n";
    cout << "Target Item : " << targetProduct << "\n";
    cout << "--------------------------------------------------\n";
    
    if (receiptLines.empty()) {
        cout << "No actions found or item selection is free.\n";
    } else {
        for (const auto& line : receiptLines) {
            cout << line << "\n";
        }
    }
    
    cout << "--------------------------------------------------\n";
    cout << "Total Optimized Cost : Rp" << (int)totalCost << "\n";
    cout << "Remaining Budget     : Rp" << (int)(budget - totalCost) << "\n";
    cout << "==================================================\n";

    if (totalCost > budget) {
        cout << "WARNING: Total cost exceeds your budget by Rp" << (int)(totalCost - budget) << "!\n";
    } else {
        cout << "SUCCESS: Solution is fully affordable.\n";
    }
    cout << "==================================================\n";

    return 0;
}
