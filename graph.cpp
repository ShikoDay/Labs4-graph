#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

// Класс для представления вершины графа
template <class V, class E>
class Vertex; // Предварительное объявление для использования в Edge

// Класс для представления ребра графа
template <class V, class E>
class Edge
{
public:
    Edge(const E &properties, Vertex<V, E> *vertex1, Vertex<V, E> *vertex2)
        : properties_(properties), vertex1_(vertex1), vertex2_(vertex2) {}

    const Vertex<V, E> *getVertex1() const { return vertex1_; }
    const Vertex<V, E> *getVertex2() const { return vertex2_; }
    const E *getProperties() const { return &properties_; }

private:
    E properties_;
    Vertex<V, E> *vertex1_;
    Vertex<V, E> *vertex2_;
};

// Класс для представления вершины графа
template <class V, class E>
class Vertex
{
public:
    Vertex(const V &properties) : properties_(properties) {}

    const V *getProperties() const { return &properties_; }
    const std::vector<Edge<V, E> *> *getEdges() const { return &edges_; }

    void addEdge(const E &properties, Vertex<V, E> *target)
    {
        Edge<V, E> *edge = new Edge<V, E>(properties, this, target);
        edges_.push_back(edge);
        target->edges_.push_back(edge);
    }

private:
    V properties_;
    std::vector<Edge<V, E> *> edges_;
};

// Посетитель для обхода графа, который предотвращает зацикливание
template <class V, class E>
class OneTimeVisitor
{
public:
    bool visitVertex(const Vertex<V, E> *vertex)
    {
        if (std::find(visited_.begin(), visited_.end(), vertex) != visited_.end())
            return false;
        visited_.push_back(vertex);
        return true;
    }

    bool visitEdge(const Edge<V, E> *) { return true; }

    void leaveVertex(const Vertex<V, E> *) { visited_.pop_back(); }

    void leaveEdge(const Edge<V, E> *) {}

private:
    std::vector<const Vertex<V, E> *> visited_;
};

// Алгоритм обхода графа в глубину с использованием паттерна "Посетитель"
template <class V, class E, class F>
void depthPass(const Vertex<V, E> *vertex, F *visitor)
{
    if (!visitor->visitVertex(vertex))
        return;

    for (Edge<V, E> *edge : *vertex->getEdges())
    {
        if (!visitor->visitEdge(edge))
            continue;

        const Vertex<V, E> *next = edge->getVertex1() == vertex || edge->getVertex1() == nullptr
                                       ? edge->getVertex2()
                                       : edge->getVertex1();
        depthPass(next, visitor);
        visitor->leaveEdge(edge);
    }

    visitor->leaveVertex(vertex);
}

// Рекурсивная функция поиска в глубину с подсчетом стоимости пути
template <class V, class E>
bool search(const Vertex<V, E> *vertex, const V &targetName, std::vector<const Vertex<V, E> *> &visited, int &cost)
{
    if (std::find(visited.begin(), visited.end(), vertex) != visited.end())
        return false;

    visited.push_back(vertex);

    if (*vertex->getProperties() == targetName)
        return true;

    for (const Edge<V, E> *edge : *vertex->getEdges())
    {
        const Vertex<V, E> *next = (edge->getVertex1() == vertex || edge->getVertex1() == nullptr)
                                       ? edge->getVertex2()
                                       : edge->getVertex1();
        if (search(next, targetName, visited, cost))
        {
            cost += *edge->getProperties();
            return true;
        }
    }

    visited.pop_back();
    return false;
}

int main()
{
    // Создание графа
    Vertex<std::string, int> a1("A1");
    Vertex<std::string, int> b1("B1");
    Vertex<std::string, int> c1("C1");
    Vertex<std::string, int> b2("B2");
    Vertex<std::string, int> e1("E1");
    Vertex<std::string, int> f1("F1");
    Vertex<std::string, int> b3("B3");

    // Добавление ребер
    a1.addEdge(10, &b1);
    b1.addEdge(20, &c1);
    a1.addEdge(15, &b2);
    c1.addEdge(30, &e1);
    e1.addEdge(25, &f1);
    b2.addEdge(40, &f1);
    a1.addEdge(10, &b3);
    b3.addEdge(12, &c1);

    // Пример поиска пути
    std::vector<const Vertex<std::string, int> *> visited;
    int cost = 0;
    if (search(&a1, "B3", visited, cost))
    {
        std::cout << "Путь найден, стоимость: " << cost << std::endl;
    }
    else
    {
        std::cout << "Путь не найден." << std::endl;
    }

    // Использование посетителя для обхода графа
    OneTimeVisitor<std::string, int> visitor;
    std::cout << "Обход графа с использованием паттерна 'Посетитель':" << std::endl;
    depthPass(&a1, &visitor);

    return 0;
}