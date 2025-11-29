#include "sha1.h"

uint cycle_shift_left(uint val, int bit_count)
{
   return (val << bit_count | val >> (32 - bit_count));
}

std::vector<uint8_t> SHA1(const std::vector<uint8_t>& password)
{
   //инициализация
   uint A = H[0];
   uint B = H[1];
   uint C = H[2];
   uint D = H[3];
   uint E = H[4];
   //подготовка данных
   std::vector<uint8_t> data = password;
   // Добавляем 1 бит (0x80)
   data.push_back(0x80);
   // Добавляем нули до кратности 56 байтам (448 бит)
   while ((data.size() % one_block_size_bytes) != 56) {
      data.push_back(0x00);
   }
   // Добавляем длину исходного сообщения в битах (64-битное значение big-endian)
   uint64_t bit_len = password.size() * 8;
   for (int i = 7; i >= 0; --i) {
      data.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF));
   }

   // Обработка блоков по 512 бит
   for (size_t i = 0; i < data.size(); i += one_block_size_bytes) {
      uint w[block_expend_size_uints];
      std::memset(w, 0, sizeof(w));

      // Копируем первые 16 слов по 4 байта
      for (int j = 0; j < one_block_size_uints; ++j) {
         w[j] = (data[i + j * 4 + 0] << 24);
         w[j] |= (data[i + j * 4 + 1] << 16);
         w[j] |= (data[i + j * 4 + 2] << 8);
         w[j] |= (data[i + j * 4 + 3]);
      }

      // Дополняем до 80 слов
      for (int j = one_block_size_uints; j < block_expend_size_uints; ++j) {
         w[j] = cycle_shift_left(w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16], 1);
      }

      uint a = A;
      uint b = B;
      uint c = C;
      uint d = D;
      uint e = E;

      for (int j = 0; j < block_expend_size_uints; ++j) {
         uint f, k;

         if (j < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
         }
         else if (j < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
         }
         else if (j < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
         }
         else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
         }

         uint temp = cycle_shift_left(a, 5) + f + e + k + w[j];
         e = d;
         d = c;
         c = cycle_shift_left(b, 30);
         b = a;
         a = temp;
      }

      A += a;
      B += b;
      C += c;
      D += d;
      E += e;
   }

   // Вывод результата в вектор байтов
   std::vector<uint8_t> hash;
   const uint H_out[5] = { A, B, C, D, E };
   for (uint h : H_out) {
      hash.push_back((h >> 24) & 0xFF);
      hash.push_back((h >> 16) & 0xFF);
      hash.push_back((h >> 8) & 0xFF);
      hash.push_back(h & 0xFF);
   }
   return hash;
}
