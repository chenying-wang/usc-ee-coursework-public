#ifndef __EE669_VAL_NODE
#define __EE669_VAL_NODE

#include <string>

#include "Node.h"

namespace ee669 {
    template<typename T>
    class ValNode : public Node<T> {
    private:
        const T val;

    public:
        ValNode(const std::string &key, const T val);
        virtual ~ValNode();

        T *eval(Session *session) const override;
    };

    template<typename T>
    ValNode<T>::ValNode(const std::string &key, const T val) :
        Node<T>(key), val(val) {}

    template<typename T>
    ValNode<T>::~ValNode() {}

    template<typename T>
    T *
    ValNode<T>::eval(Session *) const {
        return new T(this->val);
    }
}

#endif
