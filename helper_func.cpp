#pragma once
#include<bits/stdc++.h>

// Структура, що представляє ребро графа
struct Edge {
    int dest;       // вершина-призначення
    int weight;     // вага ребра
};

// Структура для пріоритетної черги
struct Node {
    int vertex;     // номер вершини
    int distance;   // відстань від початкової вершини

    // Оператор порівняння для пріоритетної черги (мінімальна відстань)
    bool operator>(const Node& other) const {
        return distance > other.distance;
    }
};

// Функція для читання графа з файлу
std::vector<std::vector<Edge>> readGraphFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    
    if (!inFile.is_open()) {
        std::cerr << "Не вдалося відкрити файл: " << filename << std::endl;
        exit(1);
    }

    int numVertices, numEdges;
    inFile >> numVertices >> numEdges;

    std::vector<std::vector<Edge>> graph(numVertices);

    for(int i = 0; i < numEdges; i++)
    {
        int src, dest, weight;
        inFile >> src >> dest >> weight;
            graph[src].push_back({dest, weight});
    }

    inFile.close();
    return graph;
}