#include "Node.h"

using ee669::NodeBase;
using ee669::Session;

NodeBase::NodeBase(const std::string &key) : key(key) {}

NodeBase::~NodeBase() {}

Session::Session() {}

Session::~Session() {
    this->release();
}

void
Session::release() {
    this->context.clear();
}
