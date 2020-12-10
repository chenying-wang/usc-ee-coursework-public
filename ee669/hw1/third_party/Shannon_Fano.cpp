/*
 * Compress text file by using Shannon-Fano (No real compression,
 * just get the 0101 code)
 *
 * Author: bonep
 * Last update: 20120425
 *
 * Please free feel to use this code in any way
 *
 * Compile:
 * linux> c++ Shannon_Fano.cpp
 *
 * Usage:
 * linux> ./a.out
 * (the input text file is hardcoded as text1.txt)
 */
#include <iostream>
#include <string>
#include <vector>

#include "ShannonFano.h"

third_party::SymbolCode::SymbolCode(char in_symbol, int in_frequency) {
    symbol = in_symbol;
    frequency = in_frequency;
    code = "";
}

char third_party::SymbolCode::getSymbol() {
    return symbol;
}

int third_party::SymbolCode::getFrequency() {
    return frequency;
}

std::string third_party::SymbolCode::getCode() {
    return code;
}

void third_party::SymbolCode::addCode(std::string in_code) {
    code += in_code;
}

void third_party::shannonFano(std::vector<SymbolCode> &symbol, int in_begin, int in_end) {
    if ( in_begin >= in_end ) {
        return;
    }

    if ( in_begin == in_end - 1 ) {
        symbol.at( in_begin ).addCode( "0" );
        symbol.at( in_end ).addCode( "1" );
        return;
    }

    int highPtr = in_begin;
    int lowPtr = in_end;
    int highSum = symbol.at( highPtr ).getFrequency();
    int lowSum = symbol.at( lowPtr ).getFrequency();

    while ( highPtr != lowPtr - 1 ) {
        if ( highSum > lowSum ) {
            lowPtr --;
            lowSum += symbol.at( lowPtr ).getFrequency();
        } else {
            highPtr ++;
            highSum += symbol.at( highPtr ).getFrequency();
        }
    }

    int i;
    for ( i=in_begin; i<=highPtr; i++ ) {
        symbol.at( i ).addCode( "0" );
    }
    for ( i=lowPtr; i<=in_end; i++ ) {
        symbol.at( i ).addCode( "1" );
    }

    shannonFano( symbol, in_begin, highPtr );
    shannonFano( symbol, lowPtr, in_end );
}
