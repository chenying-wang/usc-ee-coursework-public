#ifndef __EE669_GRAPH
#define __EE669_GRAPH

#include <vector>

#include "Node.h"

namespace ee669 {
    template<typename T>
    class Graph {
    private:
        std::vector<NodeBase *> nodes;

    protected:
        template<typename R>
        R *addNode(R *node);

    public:
        Graph();
        virtual ~Graph();

        virtual ee669::Node<T> *createGraph() = 0;

        void deleteGraph();
    };

    template<typename T>
    Graph<T>::Graph() {}

    template<typename T>
    Graph<T>::~Graph() {
        this->deleteGraph();
    }

    template<typename T>
    void
    Graph<T>::deleteGraph() {
        for (auto node : this->nodes) {
            delete node;
        }
        this->nodes.clear();
        this->nodes.shrink_to_fit();
    }

    template<typename T>
    template<typename R>
    R *
    Graph<T>::addNode(R *node) {
        if (node != nullptr) {
            this->nodes.push_back(node);
        }
        return node;
    }
}

#endif
