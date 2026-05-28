#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <limits>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// ---------------------------------------------------------
// DATA STRUCTURES
// ---------------------------------------------------------
struct Edge
{
    string destination;
    string actionDescription;
    double cost;
};

// ---------------------------------------------------------
// CLASS: CulinaryGraph
// Handles the Adjacency List representing the supply chain
// ---------------------------------------------------------
class CulinaryGraph
{
public:
    unordered_map<string, vector<Edge>> adjList;

    void addEdge(const string &source, const string &dest, const string &action, double cost)
    {
        adjList[source].push_back({dest, action, cost});
    }
};

// ---------------------------------------------------------
// CLASS: CSVParser
// Handles data ingestion from the market_prices.csv file
// ---------------------------------------------------------
class CSVParser
{
public:
    static bool loadGraph(const string &filename, CulinaryGraph &graph)
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            cout << "ERROR: Could not open " << filename << "\n";
            return false;
        }

        string line, source, dest, action, costStr;
        getline(file, line); // Skip header

        while (getline(file, line))
        {
            stringstream ss(line);
            getline(ss, source, ',');
            getline(ss, dest, ',');
            getline(ss, action, ',');
            getline(ss, costStr, ',');

            if (!costStr.empty() && costStr.back() == '\r')
            {
                costStr.pop_back();
            }

            if (!source.empty() && !dest.empty() && !costStr.empty())
            {
                try
                {
                    graph.addEdge(source, dest, action, stod(costStr));
                }
                catch (...)
                {
                    continue;
                }
            }
        }
        file.close();
        return true;
    }
};

// ---------------------------------------------------------
// CLASS: GraphvizExporter
// Generates professional DOT files for network visualization
// ---------------------------------------------------------
class GraphvizExporter
{
public:
    static void generateDOTFile(
        CulinaryGraph &graph,
        const string &filename,
        unordered_map<string, string> &parentNode,
        const string &startNode,
        const string &targetNode,
        bool pathFound)
    {
        ofstream file(filename);
        if (!file.is_open())
        {
            cout << "WARNING: Could not generate visualization file.\n";
            return;
        }

        // Initialize DOT format headers
        file << "digraph CulinarySupplyChain {\n";
        file << "    rankdir=LR;\n"; // Force layout from Left to Right
        file << "    node [fontname=\"Arial\", shape=box, style=\"rounded,filled\", fillcolor=\"#F3F4F6\", color=\"#D1D5DB\"];\n";
        file << "    edge [fontname=\"Arial\", fontsize=10, color=\"#9CA3AF\", fontcolor=\"#4B5563\"];\n";

        // Custom styling for entry and exit nodes
        file << "    \"" << startNode << "\" [fillcolor=\"#DBEAFE\", color=\"#3B82F6\", shape=circle, penwidth=2.0];\n";
        if (pathFound)
        {
            file << "    \"" << targetNode << "\" [fillcolor=\"#D1FAE5\", color=\"#10B981\", shape=doublecircle, penwidth=2.0];\n";
        }

        // Map out the optimal path backwards to easily identify active edges
        unordered_map<string, string> shortestPathEdges; // stores parent -> child relationship
        if (pathFound)
        {
            string curr = targetNode;
            while (curr != startNode && parentNode.find(curr) != parentNode.end())
            {
                string parent = parentNode[curr];
                shortestPathEdges[parent] = curr;
                curr = parent;
            }
        }

        // Write all edges from our adjacency list into the DOT schema
        for (const auto &pair : graph.adjList)
        {
            string u = pair.first;
            for (const auto &edge : pair.second)
            {
                string v = edge.destination;

                file << "    \"" << u << "\" -> \"" << v << "\" [label=\""
                     << edge.actionDescription << "\\nRp " << (int)edge.cost << "\"";

                // Highlight the edge if it matches our Dijkstra shortest path trace
                if (shortestPathEdges.find(u) != shortestPathEdges.end() && shortestPathEdges[u] == v)
                {
                    file << ", color=\"#10B981\", penwidth=3.5, fontcolor=\"#047857\"";
                }

                file << "];\n";
            }
        }

        file << "}\n";
        file.close();
        cout << ">>> Visual graph schema generated successfully: '" << filename << "'\n";
    }
};

// ---------------------------------------------------------
// CLASS: DijkstraSolver
// The core algorithmic engine with built-in visualization hooks
// ---------------------------------------------------------
class DijkstraSolver
{
public:
    static void findCheapestPath(CulinaryGraph &graph, const string &startNode, const string &targetNode)
    {
        unordered_map<string, double> minCost;
        unordered_map<string, string> parentNode;
        unordered_map<string, Edge> parentEdge;

        priority_queue<pair<double, string>, vector<pair<double, string>>, greater<pair<double, string>>> pq;

        for (const auto &pair : graph.adjList)
        {
            minCost[pair.first] = numeric_limits<double>::infinity();
            for (const auto &edge : pair.second)
            {
                minCost[edge.destination] = numeric_limits<double>::infinity();
            }
        }

        if (minCost.find(targetNode) == minCost.end())
        {
            cout << "\n==================================================\n";
            cout << "                 ERROR OCCURRED                   \n";
            cout << "==================================================\n";
            cout << "ERROR: '" << targetNode << "' was not found in the dataset.\n\n";
            cout << "Available products detected in your CSV:\n";
            for (const auto &pair : minCost)
            {
                if (pair.first != startNode)
                    cout << " - " << pair.first << "\n";
            }
            cout << "==================================================\n";
            return;
        }

        minCost[startNode] = 0;
        pq.push({0.0, startNode});

        while (!pq.empty())
        {
            double currentCost = pq.top().first;
            string u = pq.top().second;
            pq.pop();

            if (currentCost > minCost[u])
                continue;
            if (u == targetNode)
                break;

            for (const auto &edge : graph.adjList[u])
            {
                string v = edge.destination;
                double nextCost = currentCost + edge.cost;

                if (nextCost < minCost[v])
                {
                    minCost[v] = nextCost;
                    parentNode[v] = u;
                    parentEdge[v] = edge;
                    pq.push({nextCost, v});
                }
            }
        }

        bool pathFound = (minCost[targetNode] != numeric_limits<double>::infinity());

        // Print receipt to console
        printOptimalReceipt(minCost, parentNode, parentEdge, startNode, targetNode, pathFound);

        // Automatically generate visual graph export
        GraphvizExporter::generateDOTFile(graph, "culinary_map.dot", parentNode, startNode, targetNode, pathFound);
    }

private:
    static void printOptimalReceipt(
        unordered_map<string, double> &minCost,
        unordered_map<string, string> &parentNode,
        unordered_map<string, Edge> &parentEdge,
        const string &startNode,
        const string &targetNode,
        bool pathFound)
    {
        cout << "\n==================================================\n";
        cout << "                 OPTIMAL RECEIPT                  \n";
        cout << "==================================================\n";
        cout << "Target Item : " << targetNode << "\n";
        cout << "--------------------------------------------------\n";

        if (!pathFound)
        {
            cout << "ERROR: No manufacturing path could be found to reach this item.\n";
            cout << "==================================================\n";
            return;
        }

        vector<string> receiptLines;
        string curr = targetNode;
        int loopGuard = 0;

        while (curr != startNode)
        {
            if (parentNode.find(curr) == parentNode.end() || loopGuard > 500)
            {
                cout << "ERROR: Path reconstruction failed due to broken database mappings.\n";
                cout << "==================================================\n";
                return;
            }
            string p = parentNode[curr];
            Edge e = parentEdge[curr];

            string line = "- Step: " + e.actionDescription + " (Yields: " + curr + ") -> Rp " + to_string((int)e.cost);
            receiptLines.push_back(line);
            curr = p;
            loopGuard++;
        }

        reverse(receiptLines.begin(), receiptLines.end());

        for (size_t i = 0; i < receiptLines.size(); ++i)
        {
            cout << (i + 1) << ". " << receiptLines[i] << "\n";
        }

        cout << "--------------------------------------------------\n";
        cout << "Total Optimized Cost : Rp " << (int)minCost[targetNode] << "\n";
        cout << "==================================================\n";
    }
};

// ---------------------------------------------------------
// MAIN RUNTIME EXECUTABLE
// ---------------------------------------------------------
int main()
{
    cout << "==================================================\n";
    cout << "        CULINARY GRAPH: DIJKSTRA SYSTEM           \n";
    cout << "==================================================\n";

    CulinaryGraph supplyChain;

    cout << "Loading market data from CSV...\n";
    if (!CSVParser::loadGraph("market_prices.csv", supplyChain))
    {
        cout << "Execution halted due to missing database resource.\n";
        return 1;
    }
    cout << "Data loaded successfully!\n\n";

    string targetProduct;
    cout << "Enter the exact name of the product you want to make: ";
    getline(cin, targetProduct);

    DijkstraSolver::findCheapestPath(supplyChain, "START", targetProduct);

    return 0;
}
