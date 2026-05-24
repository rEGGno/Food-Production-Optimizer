#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <limits>
#include <algorithm>

using namespace std;

struct Edge {
    int targetState;
    double cost;
    string actionDescription;
};

struct MarketOption {
    string description;
    double cost;
};

int main() {
    string bufferInput;

    cout << "==================================================\n";
    cout << "                  DIJKSTRA SYSTEM                 \n";
    cout << "==================================================\n";

    cout << "How many core ingredients does the recipe need? ";
    getline(cin, bufferInput);
    int totalIngredients = stoi(bufferInput);

    if (totalIngredients > 15 || totalIngredients <= 0) {
        cout << "Please enter a value between 1 and 15.\n";
        return 1;
    }

    vector<string> ingredientNames(totalIngredients);
    vector<vector<MarketOption>> sourcingOptions(totalIngredients);

    for (int i = 0; i < totalIngredients; ++i) {
        cout << "\nEnter name for Ingredient #" << (i + 1) << " (e.g., 1 kg Flour): ";
        getline(cin, ingredientNames[i]);

        cout << "How many sourcing options/pathways exist for " << ingredientNames[i] << "? ";
        getline(cin, bufferInput);
        int totalOptions = stoi(bufferInput);

        for (int j = 0; j < totalOptions; ++j) {
            MarketOption option;
            cout << "  Option " << (j + 1) << " description (e.g., Buy from Market A): ";
            getline(cin, option.description);

            cout << "  Price/Cost for this option (Rp): ";
            getline(cin, bufferInput);
            option.cost = stod(bufferInput);

            sourcingOptions[i].push_back(option);
        }
    }

    cout << "\n==================================================\n";
    cout << "Create a final composite product name (e.g., Loaf of Bread): ";
    string customProductName;
    getline(cin, customProductName);

    cout << "Enter your available budget (Rp): ";
    getline(cin, bufferInput);
    double budget = stod(bufferInput);

    int totalStates = 1 << totalIngredients;
    int targetState = totalStates - 1;

    vector<vector<Edge>> graph(totalStates);

    for (int currentState = 0; currentState < totalStates; ++currentState) {
        for (int i = 0; i < totalIngredients; ++i) {
            if ((currentState & (1 << i)) == 0) {
                int nextState = currentState | (1 << i);
                
                for (const auto& option : sourcingOptions[i]) {
                    graph[currentState].push_back({nextState, option.cost, option.description});
                }
            }
        }
    }

    vector<double> minCost(totalStates, numeric_limits<double>::infinity());
    vector<int> parentState(totalStates, -1);
    vector<Edge> parentEdge(totalStates);

    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;

    minCost[0] = 0;
    pq.push({0.0, 0});

    while (!pq.empty()) {
        double currentCost = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (currentCost > minCost[u]) continue;

        if (u == targetState) break;

        for (const auto& edge : graph[u]) {
            int v = edge.targetState;
            double nextCost = currentCost + edge.cost;

            if (nextCost < minCost[v]) {
                minCost[v] = nextCost;
                parentState[v] = u;
                parentEdge[v] = edge;
                pq.push({nextCost, v});
            }
        }
    }

    double totalCost = minCost[targetState];
    vector<string> receiptLines;
    
    int curr = targetState;
    while (curr != 0 && parentState[curr] != -1) {
        receiptLines.push_back("- " + parentEdge[curr].actionDescription + " : Rp" + to_string((int)parentEdge[curr].cost));
        curr = parentState[curr];
    }
    reverse(receiptLines.begin(), receiptLines.end());


    cout << "\n==================================================\n";
    cout << "                  OPTIMAL RECEIPT                   \n";
    cout << "====================================================\n";
    cout << "Target Item : " << customProductName << "\n";
    cout << "--------------------------------------------------\n";
    
    if (totalCost == numeric_limits<double>::infinity() || receiptLines.empty()) {
        cout << "No manufacturing path could be processed.\n";
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
