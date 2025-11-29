#pragma once
#include<iostream>
#include<vector>
#include<cstdint>

typedef uint32_t uint;
constexpr size_t one_block_size_bytes = 64; //количество байт в блоке - размер одного блока 512 бит
constexpr size_t one_block_size_uints = 16; //сколько 4-байтовых чисел(uint) в одном блоке (64\4 = 16)
constexpr size_t block_expend_size_uints = 80; //сколько 4-байтовых чисел в расширенном блоке (после расширения до 80 слов)

//константы согласно стандарту
const uint H[5]{
   0x67452301,
   0xEFCDAB89,
   0x98BADCFE,
   0x10325476,
   0xC3D2E1F0
};

//циклический сдвиг влево
uint cycle_shift_left(uint val, int bit_count);
//основная функция sha1
std::vector<uint8_t> SHA1(const std::vector<uint8_t>& password);