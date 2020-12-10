#ifndef __EE669_SERIALIZATION_NODE
#define __EE669_SERIALIZATION_NODE

#include <vector>
#include <string>

#include "common/Matrix.h"
#include "Node.h"

namespace ee669 {
    template<typename T>
    class SerializationNode : public Node<std::vector<T>> {
    private:
        const std::string inputNode;

    public:
        SerializationNode(const std::string &key, const std::string &inputNode);
        virtual ~SerializationNode();

        std::vector<T> *eval(Session *session) const override;
    };

    template<typename T>
    SerializationNode<T>::SerializationNode(const std::string &key, const std::string &inputNode) :
        Node<std::vector<T>>(key), inputNode(inputNode) {}

    template<typename T>
    SerializationNode<T>::~SerializationNode() {}

    template<typename T>
    std::vector<T> *
    SerializationNode<T>::eval(Session *session) const {
        auto input = this->template getDependency<ee669::Matrix<T>>(session, this->inputNode);
        auto result = new std::vector<T>();
        result->insert(result->end(), input->getData().cbegin(), input->getData().cend());
        return result;
    }
}

#endif
