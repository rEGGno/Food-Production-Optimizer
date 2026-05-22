#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <limits>
#include <algorithm>
#include <iomanip>

using namespace std;

// Structure to represent a recipe option/pathway
struct Edge {
    int targetNode;
    double cost;
    string actionDescription;
};

// Global structures to map string names to integer IDs for the graph
unordered_map<string, int> nodeIDs;
unordered_map<int, string> nodeNames;
int nodeCounter = 0;

// Function to safely get or create a node ID
int getNodeID(const string& name) {
    if (nodeIDs.find(name) == nodeIDs.end()) {
        nodeIDs[name] = nodeCounter;
        nodeNames[nodeCounter] = name;
        nodeCounter++;
    }
    return nodeIDs[name];
}

int main() {
    // 1. Initialize Graph Adjacency List
    // graph[u] holds all ways to produce/sustain node 'u' from pre-requisite components
    // Because Dijkstra finds the path from a source to a destination, we will build the graph
    // backwards: Edges point from the END product, down through ingredients, to the RAW starting points.
    unordered_map<int, vector<Edge>> graph;

    // --- POPULATE DATA BASED ON PROPOSAL EXAMPLES ---
    
    // Butter Pathways
    int butter = getNodeID("1 kg Butter");
    graph[butter].push_back({getNodeID("Supermarket Premium Butter"), 75000, "Buy premium packaged butter at Supermarket"});
    graph[butter].push_back({getNodeID("Wholesale Distributor Butter"), 50000, "Buy bulk butter from Wholesale Distributor"});
    graph[butter].push_back({getNodeID("Home Churned Butter"), 10000, "Churn dairy at home (Labor & Electricity)"});

    // Home Churned Butter requires Raw Dairy
    int homeChurned = getNodeID("Home Churned Butter");
    graph[homeChurned].push_back({getNodeID("Local Farm Dairy"), 20000, "Purchase raw dairy from Local Farm"});

    // Flour Pathways
    int flour = getNodeID("1 kg Flour");
    graph[flour].push_back({getNodeID("Market A Flour"), 12000, "Buy flour from Market A"});
    graph[flour].push_back({getNodeID("Market B Flour"), 15000, "Buy flour from Market B"});
    graph[flour].push_back({getNodeID("Market C Flour"), 14000, "Buy flour from Market C"});

    // Loaf of Bread Pathway (Requires Butter and Flour)
    int bread = getNodeID("Loaf of Bread");
    graph[bread].push_back({butter, 0, "Prepare Butter component"});
    graph[bread].push_back({flour, 0, "Prepare Flour component"});

    // Base raw materials cost 0 to "produce" since they are the ultimate leaves
    graph[getNodeID("Supermarket Premium Butter")] = {};
    graph[getNodeID("Wholesale Distributor Butter")] = {};
    graph[getNodeID("Local Farm Dairy")] = {};
    graph[getNodeID("Market A Flour")] = {};
    graph[getNodeID("Market B Flour")] = {};
    graph[getNodeID("Market C Flour")] = {};


    // 2. User Input Interface
    cout << "==================================================\n";
    cout << "       BAKERY SUPPLY PATHWAY OPTIMIZER            \n";
    cout << "==================================================\n";
    
    cout << "Available target products to optimize: \n";
    cout << " - Loaf of Bread\n";
    cout << " - 1 kg Butter\n";
    cout << " - 1 kg Flour\n\n";

    cout << "Enter the product you want to make: ";
    string targetProduct;
    getline(cin, targetProduct);

    if (nodeIDs.find(targetProduct) == nodeIDs.end()) {
        cout << "\nError: Product not found in recipe database.\n";
        return 1;
    }

    cout << "Enter your available budget (Rp): ";
    double budget;
    cin >> budget;


    // 3. Dijkstra's Algorithm implementation
    int startNode = nodeIDs[targetProduct];
    
    // Track minimum costs and parent pointers for path reconstruction
    vector<double> minCost(nodeCounter, numeric_limits<double>::infinity());
    vector<int> parentNode(nodeCounter, -1);
    vector<Edge> parentEdge(nodeCounter);

    // Priority queue storing pair<cost, node_id>
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;

    minCost[startNode] = 0;
    pq.push({0, startNode});

    while (!pq.empty()) {
        double currentCost = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (currentCost > minCost[u]) continue;

        // Explore alternative pathway branches
        for (const auto& edge : graph[u]) {
            int v = edge.targetNode;
            double nextCost = currentCost + edge.cost;

            if (nextCost < minCost[v]) {
                minCost[v] = nextCost;
                parentNode[v] = u;
                parentEdge[v] = edge;
                pq.push({nextCost, v});
            }
        }
    }


    // 4. Trace graph leaves to find all necessary base actions
    double totalCost = 0;
    vector<string> receiptLines;

    // Traverse all nodes to find which structural actions were chosen
    for (int i = 0; i < nodeCounter; ++i) {
        // If a node was visited and it acts as a choice leading to a resource
        if (parentNode[i] != -1 && graph[i].empty()) {
            int curr = i;
            // Trace back up to aggregate costs for base-level decisions
            while (curr != startNode && parentNode[curr] != -1) {
                if (parentEdge[curr].cost > 0) {
                    string line = "- " + parentEdge[curr].actionDescription + " : Rp" + to_string((int)parentEdge[curr].cost);
                    // Avoid duplicate logs if multiple recipes share a leaf
                    if (find(receiptLines.begin(), receiptLines.end(), line) == receiptLines.end()) {
                        receiptLines.push_back(line);
                        totalCost += parentEdge[curr].cost;
                    }
                }
                curr = parentNode[curr];
            }
        }
    }

    // Handle single-item direct optimization queries (e.g. optimizing just "1 kg Butter")
    if (receiptLines.empty() && minCost[startNode] == 0) {
         for (int i = 0; i < nodeCounter; ++i) {
             if (parentNode[i] == startNode) {
                 receiptLines.push_back("- " + parentEdge[i].actionDescription + " : Rp" + to_string((int)parentEdge[i].cost));
                 totalCost += parentEdge[i].cost;
             }
         }
    }


    // 5. Output Results (Receipt Generation)
    cout << "\n==================================================\n";
    cout << "                OPTIMAL RECEIPT                   \n";
    cout << "==================================================\n";
    cout << "Target Item : " << targetProduct << "\n";
    cout << "--------------------------------------------------\n";
    
    if (receiptLines.empty()) {
        cout << "No manufacturing path found or item is free.\n";
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
