#include <iostream>  // Подключение библиотеки для ввода-вывода
#include <vector>    // Подключение библиотеки для работы с динамическими массивами (векторами)
#include <string>    // Подключение библиотеки для работы со строками
#include <algorithm> // Подключение библиотеки для алгоритмов, таких как find
#include <windows.h> // Подключение библиотеки для работы с Windows API (нужно для установки кодировки UTF-8 в PowerShel или CMD)

using namespace std;

// Класс для представления вершины графа
template <class V, class E>
class Vertex; // Предварительное объявление класса Vertex для использования в классе Edge

// Класс для представления ребер графа
template <class V, class E>
class Edge
{
public:
    // Конструктор класса Edge
    // properties - свойства ребра, vertex1 и vertex2 - указатели на вершины, соединенные этим ребром
    Edge(const E &properties, Vertex<V, E> *vertex1, Vertex<V, E> *vertex2)
        : properties_(properties), vertex1_(vertex1), vertex2_(vertex2) {}

    // Метод для получения первой вершины, связанной с ребром
    const Vertex<V, E> *getVertex1() const { return vertex1_; }

    // Метод для получения второй вершины, связанной с ребром
    const Vertex<V, E> *getVertex2() const { return vertex2_; }

    // Метод для получения свойств ребра
    const E *getProperties() const { return &properties_; }

private:
    E properties_;          // Свойства ребра
    Vertex<V, E> *vertex1_; // Указатель на первую вершину
    Vertex<V, E> *vertex2_; // Указатель на вторую вершину
};

// Класс для представления вершины графа
template <class V, class E>
class Vertex
{
public:
    // Метод для вывода информации о вершине и её рёбрах
    void print() const
    {
        cout << "Вершина: " << properties_ << endl;
        for (const Edge<V, E> *edge : edges_)
        {
            cout << "  Ребро к " << *(edge->getVertex2()->getProperties())
                 << " (стоимость: " << *(edge->getProperties()) << ")" << endl;
        }
    }
    // Конструктор класса Vertex
    // properties - свойства вершины
    Vertex(const V &properties) : properties_(properties) {}

    // Метод для получения свойств вершины
    const V *getProperties() const { return &properties_; }

    // Метод для получения списка ребер, связанных с вершиной
    const vector<Edge<V, E> *> *getEdges() const { return &edges_; }

    // Метод для добавления ребра, соединяющего эту вершину с целевой вершиной
    void addEdge(const E &properties, Vertex<V, E> *target)
    {
        // Создание нового ребра
        Edge<V, E> *edge = new Edge<V, E>(properties, this, target);
        edges_.push_back(edge);         // Добавление ребра в список этой вершины
        target->edges_.push_back(edge); // Добавление ребра в список целевой вершины
    }

private:
    V properties_;               // Свойства вершины
    vector<Edge<V, E> *> edges_; // Список указателей на ребра, связанные с вершиной
};

// Посетитель для обхода графа, который предотвращает зацикливание
template <class V, class E>
class OneTimeVisitor
{
public:
    // Метод для посещения вершины
    // Возвращает true, если вершина была посещена впервые, иначе false
    bool visitVertex(const Vertex<V, E> *vertex)
    {
        if (find(visited_.begin(), visited_.end(), vertex) != visited_.end())
            return false;           // Если вершина уже посещена, возвращаем false
        visited_.push_back(vertex); // Добавляем вершину в список посещенных
        return true;                // Возвращаем true, так как вершина посещена
    }

    // Метод для посещения ребра
    // Возвращает true, так как мы не ограничиваем посещение ребер
    bool visitEdge(const Edge<V, E> *) { return true; }

    // Метод для выхода из вершины
    void leaveVertex(const Vertex<V, E> *) { visited_.pop_back(); } // Удаляем последнюю посещенную вершину

    // Метод для выхода из ребра
    void leaveEdge(const Edge<V, E> *) {} // Ничего не делаем при выходе из ребра

private:
    vector<const Vertex<V, E> *> visited_; // Список посещенных вершин
};

// Алгоритм обхода графа в глубину с использованием паттерна "Посетитель"
// vertex - указатель на текущую вершину графа, которую мы обходим
// visitor - указатель на объект посетителя, который реализует логику обработки вершин и ребер
template <class V, class E, class F>
void depthPass(const Vertex<V, E> *vertex, F *visitor)
{
    // Если посетитель не разрешает посещение текущей вершины, выходим из функции
    if (!visitor->visitVertex(vertex))
        return;

    // Проходим по всем рёбрам текущей вершины
    for (Edge<V, E> *edge : *vertex->getEdges())
    {
        // Если посетитель не разрешает обработку текущего ребра, переходим к следующему
        if (!visitor->visitEdge(edge))
            continue;

        // Определяем следующую вершину для обхода
        const Vertex<V, E> *next = edge->getVertex1() == vertex || edge->getVertex1() == nullptr
                                       ? edge->getVertex2()
                                       : edge->getVertex1();

        // Рекурсивно вызываем обход для следующей вершины
        depthPass(next, visitor);

        // Уведомляем посетителя о том, что покидаем текущее ребро
        visitor->leaveEdge(edge);
    }

    // Уведомляем посетителя о том, что покидаем текущую вершину
    visitor->leaveVertex(vertex);
}

// Рекурсивная функция поиска в глубину с подсчетом стоимости пути
// vertex - указатель на текущую вершину, которую мы исследуем
// targetName - имя целевой вершины, которую мы ищем
// visited - вектор, содержащий посещенные вершины, чтобы избежать циклов
// cost - ссылка на переменную, в которую будет записана стоимость найденного пути
template <class V, class E>
bool search(const Vertex<V, E> *vertex, const V &targetName, vector<const Vertex<V, E> *> &visited, int &cost)
{
    // Проверяем, была ли текущая вершина уже посещена
    if (find(visited.begin(), visited.end(), vertex) != visited.end())
        return false;

    // Добавляем текущую вершину в список посещенных
    visited.push_back(vertex);

    // Проверяем, является ли текущая вершина целевой
    if (*vertex->getProperties() == targetName)
    {
        cout << "Целевая вершина " << targetName << " найдена!" << endl;
        return true;
    }

    // Проходим по всем рёбрам текущей вершины
    for (const Edge<V, E> *edge : *vertex->getEdges())
    {
        // Определяем следующую вершину для обхода
        const Vertex<V, E> *next = (edge->getVertex1() == vertex || edge->getVertex1() == nullptr)
                                       ? edge->getVertex2()
                                       : edge->getVertex1();

        // Выводим информацию о текущем шаге
        cout << "Переход к вершине " << *(next->getProperties())
             << " через ребро со стоимостью " << *(edge->getProperties()) << endl;

        // Рекурсивно вызываем поиск для следующей вершины
        if (search(next, targetName, visited, cost))
        {
            // Если путь найден, добавляем стоимость текущего ребра к общей стоимости
            cost += *edge->getProperties();
            cout << "Добавляем стоимость ребра: " << *next->getProperties()
                 << " (стоимость: " << *edge->getProperties()
                 << "), общая стоимость: " << cost << endl;
            return true;
        }
    }

    // Если путь не найден, удаляем текущую вершину из списка посещенных
    visited.pop_back();
    return false;
}

int main()
{
    // Устанавливаем кодовую страницу консоли на UTF-8
    SetConsoleOutputCP(CP_UTF8);
    // Создание графа
    Vertex<string, int> a1("A1");
    Vertex<string, int> b1("B1");
    Vertex<string, int> c1("C1");
    Vertex<string, int> b2("B2");
    Vertex<string, int> e1("E1");
    Vertex<string, int> f1("F1");
    Vertex<string, int> b3("B3");

    // Добавление рёбер с указанием стоимости
    a1.addEdge(10, &b1);
    b1.addEdge(20, &c1);
    a1.addEdge(15, &b2);
    c1.addEdge(30, &e1);
    e1.addEdge(25, &f1);
    b2.addEdge(40, &f1);
    a1.addEdge(10, &b3);
    b3.addEdge(12, &c1);

    // Вызов метода print для каждой вершины
    a1.print();
    b1.print();
    c1.print();
    b2.print();
    e1.print();
    f1.print();
    b3.print();

    // Пример поиска пути
    vector<const Vertex<string, int> *> visited; // Вектор для хранения посещенных вершин
    int cost = 0;                                // Переменная для хранения стоимости пути

    // Использование посетителя для обхода графа
    OneTimeVisitor<string, int> visitor;
    cout << "Обход графа с использованием OneTimeVisitor:" << endl;
    depthPass(&a1, &visitor);

    // Передаею строку как string
    if (search(&a1, string("E1"), visited, cost))
    {
        cout << "Путь найден, стоимость: " << cost << endl;
    }
    else
    {
        cout << "Путь не найден." << endl;
    }

    return 0;
}

// g++ graph.cpp -o graph
//./graph