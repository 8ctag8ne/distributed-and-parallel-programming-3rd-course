#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <chrono>
#include <string>
#include "helper_func.cpp"

// Алгоритм Дейкстри з використанням priority_queue
std::vector<int> dijkstra(const std::vector<std::vector<Edge>>& graph, int startVertex) {
    int numVertices = graph.size();
    std::vector<int> distances(numVertices, std::numeric_limits<int>::max());
    std::vector<bool> visited(numVertices, false);
    
    // Пріоритетна черга з мінімальним елементом вгорі
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    
    // Відстань до початкової вершини = 0
    distances[startVertex] = 0;
    pq.push({startVertex, 0});
    
    while (!pq.empty()) {
        // Вибираємо вершину з мінімальною відстанню
        int u = pq.top().vertex;
        pq.pop();
        
        // Якщо вершина вже оброблена, пропускаємо
        if (visited[u]) continue;
        
        // Позначаємо вершину як відвідану
        visited[u] = true;
        
        // Оновлюємо відстані до суміжних вершин
        for (const auto& edge : graph[u]) {
            int v = edge.dest;
            int weight = edge.weight;
            
            // Релаксація: якщо знайдено коротший шлях
            if (!visited[v] && distances[u] != std::numeric_limits<int>::max() && 
                distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                pq.push({v, distances[v]});
            }
        }
    }
    
    return distances;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Використання: " << argv[0] << " <назва_файлу_з_графом> <початкова_вершина>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    int startVertex = std::stoi(argv[2]);

    // Зчитуємо граф з файлу
    auto graph = readGraphFromFile(filename);

    if (startVertex < 0 || startVertex >= graph.size()) {
        std::cerr << "Неправильний номер початкової вершини. Має бути від 0 до " << graph.size() - 1 << std::endl;
        return 1;
    }

    // Вимірюємо час виконання
    auto start = std::chrono::high_resolution_clock::now();
    
    // Запускаємо алгоритм Дейкстри
    auto distances = dijkstra(graph, startVertex);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    // Виводимо результати
    std::cout << "Послідовний алгоритм Дейкстри:" << std::endl;
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