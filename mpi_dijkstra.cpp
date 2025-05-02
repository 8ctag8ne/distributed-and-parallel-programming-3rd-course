#include <mpi.h>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <chrono>
#include "helper_func.cpp"  // Include your provided header file

using namespace std;
using namespace std::chrono;

const int INF = numeric_limits<int>::max();

// Function to perform Dijkstra's algorithm on a subset of vertices
void localDijkstra(const vector<vector<Edge>>& graph, vector<int>& dist, int start, int end, int source) {
    priority_queue<Node, vector<Node>, greater<Node>> pq;
    
    // Initialize distances for the local portion
    for (int i = start; i < end; i++) {
        if (i == source) {
            dist[i] = 0;
            pq.push({i, 0});
        } else {
            dist[i] = INF;
        }
    }

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        int u = current.vertex;
        if (current.distance > dist[u]) continue;

        for (const Edge& edge : graph[u]) {
            int v = edge.dest;
            int weight = edge.weight;
            
            // Only process if the destination is within our local range
            if (v >= start && v < end) {
                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    pq.push({v, dist[v]});
                }
            }
        }
    }
}

// Parallel Dijkstra implementation with timing
vector<int> parallelDijkstra(const vector<vector<Edge>>& graph, int source, double& executionTime) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int numVertices = graph.size();
    vector<int> globalDist(numVertices, INF);
    
    // Start timing (synchronize all processes first)
    MPI_Barrier(MPI_COMM_WORLD);
    auto startTime = high_resolution_clock::now();

    // Calculate workload distribution
    int chunkSize = numVertices / size;
    int remainder = numVertices % size;
    
    int start = rank * chunkSize + min(rank, remainder);
    int end = start + chunkSize + (rank < remainder ? 1 : 0);

    // Local distance vector
    vector<int> localDist(numVertices, INF);
    
    // Perform local Dijkstra computation
    localDijkstra(graph, localDist, start, end, source);

    // Gather all results to root process (rank 0)
    MPI_Reduce(rank == 0 ? MPI_IN_PLACE : localDist.data(), 
               globalDist.data(), 
               numVertices, 
               MPI_INT, 
               MPI_MIN, 
               0, 
               MPI_COMM_WORLD);

    // Broadcast the final distances to all processes
    MPI_Bcast(globalDist.data(), numVertices, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform relaxation for edges that cross partitions
    bool changed;
    do {
        changed = false;
        
        for (int u = start; u < end; u++) {
            if (localDist[u] == INF) continue;
            
            for (const Edge& edge : graph[u]) {
                int v = edge.dest;
                int weight = edge.weight;
                
                if (localDist[u] + weight < localDist[v]) {
                    localDist[v] = localDist[u] + weight;
                    changed = true;
                }
            }
        }
        
        // Synchronize changes across all processes
        MPI_Allreduce(MPI_IN_PLACE, &changed, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
        MPI_Allreduce(MPI_IN_PLACE, localDist.data(), numVertices, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
        
    } while (changed);

    // End timing
    MPI_Barrier(MPI_COMM_WORLD);
    auto endTime = high_resolution_clock::now();

    // Calculate duration (only root process needs to store this)
    if (rank == 0) {
        auto duration = duration_cast<microseconds>(endTime - startTime);
        executionTime = duration.count() / 1000.0; // Convert to milliseconds
    }

    return localDist;
}

int main(int argc, char** argv) 
{
    // Initialize MPI properly
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {
        cout << "Total processes: " << size << endl;
    }

    if (argc != 3) {
        if (rank == 0) {
            cerr << "Usage: " << argv[0] << " <filename> <source_vertex>" << endl;
        }
        MPI_Finalize();
        return 1;
    }

    string filename = argv[1];
    int source = stoi(argv[2]);

    // Read graph (could be optimized to read in parallel for large graphs)
    vector<vector<Edge>> graph;
    if (rank == 0) {
        graph = readGraphFromFile(filename);
    }

    // Broadcast graph size first
    int numVertices;
    if (rank == 0) {
        numVertices = graph.size();
    }
    MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Resize graph on all processes
    if (rank != 0) {
        graph.resize(numVertices);
    }

    // Broadcast graph data
    for (int u = 0; u < numVertices; u++) {
        int numEdges;
        if (rank == 0) {
            numEdges = graph[u].size();
        }
        MPI_Bcast(&numEdges, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank != 0) {
            graph[u].resize(numEdges);
        }

        MPI_Bcast(graph[u].data(), numEdges * sizeof(Edge), MPI_BYTE, 0, MPI_COMM_WORLD);
    }

    // Run parallel Dijkstra with timing
    double executionTime = 0.0;
    vector<int> distances = parallelDijkstra(graph, source, executionTime);

    // Output results from root process
    if (rank == 0) {
        // cout << "Shortest distances from vertex " << source << ":\n";
        // for (int i = 0; i < numVertices; i++) {
        //     cout << "Vertex " << i << ": ";
        //     if (distances[i] == INF) {
        //         cout << "INF";
        //     } else {
        //         cout << distances[i];
        //     }
        //     cout << endl;
        // }
        
        cout << "\nAlgorithm execution time: " << executionTime << " milliseconds" << endl;
    }

    cout << "Process " << rank << " of " << size << " completed." << endl;
    MPI_Finalize();
    return 0;
}