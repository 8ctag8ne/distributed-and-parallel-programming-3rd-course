#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <chrono>
#include <string>
#include <omp.h>
#include "helper_func.cpp"

// Паралельний алгоритм Дейкстри з використанням OpenMP
std::vector<int> parallel_dijkstra(const std::vector<std::vector<Edge>>& graph, int startVertex, int numThreads) {
    int numVertices = graph.size();
    std::vector<int> distances(numVertices, std::numeric_limits<int>::max());
    std::vector<bool> visited(numVertices, false);

    // Пріоритетна черга
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    distances[startVertex] = 0;
    pq.push({startVertex, 0});

    omp_set_num_threads(numThreads);

    // Динамічне визначення максимальної кількості сусідів (для буферів)
    int maxNeighbors = 0;
    for (const auto& neighbors : graph) {
        if (neighbors.size() > maxNeighbors)
            maxNeighbors = neighbors.size();
    }

    int MAX_EDGES = maxNeighbors + 16; // додатковий запас
    std::vector<Node> updates(numThreads * MAX_EDGES);
    std::vector<int> update_counts(numThreads, 0);

    while (!pq.empty()) {
        int u = pq.top().vertex;
        pq.pop();

        if (visited[u]) continue;
        visited[u] = true;

        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            update_counts[tid] = 0;

            #pragma omp for schedule(static)
            for (int i = 0; i < graph[u].size(); i++) {
                int v = graph[u][i].dest;
                int weight = graph[u][i].weight;

                if (!visited[v] && distances[u] != std::numeric_limits<int>::max()) {
                    int newDist = distances[u] + weight;

                    if (newDist < distances[v]) {
                        distances[v] = newDist;
                        int idx = tid * MAX_EDGES + update_counts[tid];
                        updates[idx] = {v, newDist};
                        update_counts[tid]++;
                    }
                }
            }
        }

        for (int t = 0; t < numThreads; ++t) {
            int base = t * MAX_EDGES;
            for (int j = 0; j < update_counts[t]; ++j) {
                pq.push(updates[base + j]);
            }
        }
    }

    return distances;
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Використання: " << argv[0] << " <назва_файлу_з_графом> <початкова_вершина> <кількість_потоків>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    int startVertex = std::stoi(argv[2]);
    int numThreads = std::stoi(argv[3]);

    // Перевірка коректності введених даних
    if (numThreads <= 0) {
        std::cerr << "Кількість потоків повинна бути додатним числом" << std::endl;
        return 1;
    }

    // Зчитуємо граф з файлу
    auto graph = readGraphFromFile(filename);

    if (startVertex < 0 || startVertex >= graph.size()) {
        std::cerr << "Неправильний номер початкової вершини. Має бути від 0 до " << graph.size() - 1 << std::endl;
        return 1;
    }

    std::cout << "Паралельний алгоритм Дейкстри з OpenMP:" << std::endl;
    std::cout << "Кількість потоків: " << numThreads << std::endl;

    // Вимірюємо час виконання
    auto start = std::chrono::high_resolution_clock::now();
    
    // Запускаємо паралельний алгоритм Дейкстри
    auto distances = parallel_dijkstra(graph, startVertex, numThreads);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    // //Виводимо результати
    // std::cout << "Найкоротші відстані від вершини " << startVertex << " до інших вершин:" << std::endl;
    
    // for (int i = 0; i < distances.size(); i++) {
    //     if (distances[i] == std::numeric_limits<int>::max()) {
    //         std::cout << "Вершина " << i << ": недосяжна" << std::endl;
    //     } else {
    //         std::cout << "Вершина " << i << ": " << distances[i] << std::endl;
    //     }
    // }
    
    std::cout << "Час виконання: " << elapsed.count() << " мс" << std::endl;

    return 0;
}