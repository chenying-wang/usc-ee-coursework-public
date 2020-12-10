#ifndef __EE669_BYTE_STREAM_PROCESSING
#define __EE669_BYTE_STREAM_PROCESSING

#include <iostream>
#include <vector>

template<typename T>
class ByteStreamProcess {
private:
    const bool procSwitch;

    virtual std::vector<T> process0(std::vector<T> &input) = 0;

public:
    ByteStreamProcess(const bool procSwitch = true);
    virtual ~ByteStreamProcess();

    std::vector<T> process(std::vector<T> &input);

    bool isSwitchOn() const {
        return this->procSwitch;
    }
};

template<typename T>
ByteStreamProcess<T>::ByteStreamProcess(const bool procSwitch) : procSwitch(procSwitch) {}

template<typename T>
ByteStreamProcess<T>::~ByteStreamProcess() {}

template<typename T>
std::vector<T>
ByteStreamProcess<T>::process(std::vector<T> &input) {
    if (!this->procSwitch) {
        return input;
    }
    return this->process0(input);
}

template<typename T>
class ByteStreamProcessChain : public ByteStreamProcess<T> {
private:
    std::vector<ByteStreamProcess<T> *> processes;

    std::vector<T> process0(std::vector<T> &input) override;

public:
    ByteStreamProcessChain();
    virtual ~ByteStreamProcessChain();

    ByteStreamProcessChain<T> *addLast(ByteStreamProcess<T> *process);
};

template<typename T>
ByteStreamProcessChain<T>::ByteStreamProcessChain() : ByteStreamProcess<T>() {}

template<typename T>
ByteStreamProcessChain<T>::~ByteStreamProcessChain() {
    for (auto process : this->processes) {
        delete process;
    }
    this->processes.clear();
}

template<typename T>
ByteStreamProcessChain<T> *
ByteStreamProcessChain<T>::addLast(ByteStreamProcess<T> *process) {
    if (process->isSwitchOn()) {
        this->processes.push_back(process);
    }
    return this;
}

template<typename T>
std::vector<T>
ByteStreamProcessChain<T>::process0(std::vector<T> &input) {
    std::vector<T> result = input;
    for (auto &process : this->processes) {
        result = process->process(result);
    }
    return result;
}

#endif
