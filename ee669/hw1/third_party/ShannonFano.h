#include <iostream>
#include <vector>
#include <string>

#ifndef __EE669_THIRD_PARTY_SHANNON_FANO
#define __EE669_THIRD_PARTY_SHANNON_FANO

namespace third_party {
    class SymbolCode {
    private:
        char symbol;
        int frequency;
        std::string code;
    public:
        SymbolCode(char in_symbol, int in_frequency);
        char getSymbol();
        int getFrequency();
        std::string getCode();
        void addCode(std::string in_code);
    };

    void shannonFano(std::vector<SymbolCode> &symbol, int in_begin, int in_end);
}

#endif
