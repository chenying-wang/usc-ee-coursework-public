#ifndef __EE669_NODE
#define __EE669_NODE

#include <unordered_map>
#include <string>

namespace ee669 {
    class Session;

    class NodeBase {
    private:
        const std::string key;

    public:
        NodeBase(const std::string &key);
        NodeBase(NodeBase &node) = delete;
        NodeBase(const NodeBase &&node) = delete;
        virtual ~NodeBase();

        // TODO delete =

        std::string getKey() const {
            return this->key;
        }
    };

    template<typename T>
    class Node : public NodeBase {
    private:
        std::unordered_map<std::string, NodeBase *> dependencies;

    protected:
        template<typename R>
        R *getDependency(Session *session, const std::string &nodeKey) const;

    public:
        Node(const std::string &key);
        virtual ~Node();

        virtual T *eval(Session *session) const = 0;

        Node<T> *dependOn(NodeBase *node);
    };

    class Session {
    private:
        std::unordered_map<std::string, void *> context;

    public:
        Session();
        virtual ~Session();

        template<typename T>
        T *run(Node<T> *node);

        template<typename T>
        T *get(const std::string &nodeKey) const;

        template<typename T>
        Session *feed(const std::string &key, T *value);

        void release();
    };

    template<typename T>
    Node<T>::Node(const std::string &key) : NodeBase(key) {}

    template<typename T>
    Node<T>::~Node() {}

    template<typename T>
    Node<T> *
    Node<T>::dependOn(NodeBase *node) {
        auto nodeKey = node->getKey();
        if (this->dependencies.count(nodeKey)) {
            throw std::runtime_error("Found duplicate node key \'" + nodeKey + '\'');
        }
        this->dependencies[nodeKey] = node;
        return this;
    }

    template<typename T>
    template<typename R>
    R *
    Node<T>::getDependency(Session *session, const std::string &nodeKey) const {
        if (!this->dependencies.count(nodeKey)) {
            return session->template get<R>(nodeKey);
        }
        return session->run(reinterpret_cast<Node<R> *>(this->dependencies.at(nodeKey)));
    }

    template<typename T>
    T *
    Session::run(Node<T> *node) {
        auto nodeKey = node->getKey();
        auto result = this->template get<T>(nodeKey);
        if (result != nullptr) {
            return result;
        }
        result = node->eval(this);
        this->context[nodeKey] = result;
        return result;
    }

    template<typename T>
    T *
    Session::get(const std::string &nodeKey) const {
        return this->context.count(nodeKey) ? reinterpret_cast<T *>(this->context.at(nodeKey)) : nullptr;
    }

    template<typename T>
    Session *
    Session::feed(const std::string &key, T *value) {
        this->context[key] = value;
        return this;
    }
}

#endif
