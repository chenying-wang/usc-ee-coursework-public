#ifndef __EE669_STREAM_PROCESSING
#define __EE669_STREAM_PROCESSING

#include <iostream>
#include <vector>

namespace ee669 {
    template<typename T>
    class StreamProcess {
    private:
        const bool procSwitch;

        virtual std::vector<T> process0(std::vector<T> &input) = 0;

    public:
        StreamProcess(const bool procSwitch = true);
        virtual ~StreamProcess();

        std::vector<T> process(std::vector<T> &input);

        bool isSwitchOn() const {
            return this->procSwitch;
        }
    };

    template<typename T>
    StreamProcess<T>::StreamProcess(const bool procSwitch) : procSwitch(procSwitch) {}

    template<typename T>
    StreamProcess<T>::~StreamProcess() {}

    template<typename T>
    std::vector<T>
    StreamProcess<T>::process(std::vector<T> &input) {
        if (!this->procSwitch) {
            return input;
        }
        return this->process0(input);
    }

    template<typename T>
    class StreamProcessChain : public StreamProcess<T> {
    private:
        std::vector<StreamProcess<T> *> processes;

        std::vector<T> process0(std::vector<T> &input) override;

    public:
        StreamProcessChain();
        virtual ~StreamProcessChain();

        StreamProcessChain<T> *addLast(StreamProcess<T> *process);
    };

    template<typename T>
    StreamProcessChain<T>::StreamProcessChain() : StreamProcess<T>() {}

    template<typename T>
    StreamProcessChain<T>::~StreamProcessChain() {
        for (auto process : this->processes) {
            delete process;
        }
        this->processes.clear();
    }

    template<typename T>
    StreamProcessChain<T> *
    StreamProcessChain<T>::addLast(StreamProcess<T> *process) {
        if (process->isSwitchOn()) {
            this->processes.push_back(process);
        }
        return this;
    }

    template<typename T>
    std::vector<T>
    StreamProcessChain<T>::process0(std::vector<T> &input) {
        std::vector<T> result = input;
        for (auto &process : this->processes) {
            result = process->process(result);
        }
        return result;
    }
}

#endif
