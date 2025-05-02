#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <ctime>
#include <string>

// Структура, що представляє ребро графа
struct Edge {
    int dest;       // вершина-призначення
    int weight;     // вага ребра
};

// Функція для генерації списку суміжності графа
std::vector<std::vector<Edge>> generateGraph(int numVertices, double density) {
    // Ініціалізація генератора випадкових чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> weightDist(1, 100);  // Ваги від 1 до 100
    std::uniform_real_distribution<> probabilityDist(0.0, 1.0);  // Імовірність для визначення наявності ребра

    // Створення порожнього списку суміжності
    std::vector<std::vector<Edge>> adjList(numVertices);

    // Заповнення списку суміжності
    for (int i = 0; i < numVertices; i++) {
        for (int j = 0; j < numVertices; j++) {
            // Пропускаємо петлі (ребра до самого себе)
            if (i == j) continue;

            // Створюємо ребро з імовірністю, що відповідає заданій густині
            if (probabilityDist(gen) < density) {
                int weight = weightDist(gen);
                adjList[i].push_back({j, weight});
            }
        }
    }

    return adjList;
}

// Функція для запису графа у файл
void writeGraphToFile(const std::vector<std::vector<Edge>>& graph, const std::string& filename) {
    std::ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        std::cerr << "Не вдалося відкрити файл для запису: " << filename << std::endl;
        return;
    }

    // Спочатку записуємо кількість вершин
    outFile << graph.size() << std::endl;

    // Потім записуємо ребра
    for (int i = 0; i < graph.size(); i++) {
        for (const auto& edge : graph[i]) {
            outFile << i << " " << edge.dest << " " << edge.weight << std::endl;
        }
    }

    outFile.close();
    std::cout << "Граф успішно записано у файл: " << filename << std::endl;
}

int main(int argc, char* argv[]) {
    int numVertices;
    double density;
    std::string filename;

    // Перевірка аргументів командного рядка
    if (argc != 4) {
        std::cout << "Використання: " << argv[0] << " <кількість_вершин> <густина> <назва_файлу>" << std::endl;
        std::cout << "Введіть кількість вершин: ";
        std::cin >> numVertices;
        std::cout << "Введіть густину графа (від 0 до 1): ";
        std::cin >> density;
        std::cout << "Введіть назву файлу для запису: ";
        std::cin >> filename;
    } else {
        numVertices = std::stoi(argv[1]);
        density = std::stod(argv[2]);
        filename = argv[3];
    }

    // Перевірка коректності введених даних
    if (numVertices <= 0) {
        std::cerr << "Кількість вершин повинна бути додатним числом" << std::endl;
        return 1;
    }

    if (density < 0.0 || density > 1.0) {
        std::cerr << "Густина повинна бути в діапазоні [0, 1]" << std::endl;
        return 1;
    }

    // Генерація та запис графа
    auto graph = generateGraph(numVertices, density);
    writeGraphToFile(graph, filename);

    return 0;
}