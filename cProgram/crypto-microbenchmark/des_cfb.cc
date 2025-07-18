/******************************************************************************
 *  Copyright (c) 2015 Jamis Hoo
 *  Distributed under the MIT license 
 *  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *  
 *  Project: 
 *  Filename: des_cfb.cc 
 *  Version: 1.0
 *  Author: Jamis Hoo
 *  E-mail: hoojamis@gmail.com
 *  Date: May 15, 2015
 *  Time: 21:09:32
 *  Description: DES, Cypher Feedback Block(in 8 bit) Mode(CFB) 
 *****************************************************************************/
#include <cstring>
#include <cinttypes>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <vector>
#include <array>

inline void setbit(void* ptr, size_t i, bool val) {
    if (val) 
        ((uint8_t*)ptr)[i / 8] |=   '\x01' << (7 - i % 8);
    else 
        ((uint8_t*)ptr)[i / 8] &= ~('\x01' << (7 - i % 8));
}

inline bool getbit(const void* ptr, size_t i) {
    return (((uint8_t*)ptr)[i / 8] & ('\x01' << (7 - i % 8)))? 1: 0;
}


// key is 64 bits
std::array<std::array<uint8_t, 6>, 16> generateSubkeys(const uint8_t key[]) {
    constexpr size_t pc1[56] = { 57, 49, 41, 33, 25, 17,  9,
                                  1, 58, 50, 42, 34, 26, 18,
                                 10,  2, 59, 51, 43, 35, 27, 
                                 19, 11,  3, 60, 52, 44, 36, 
                                 63, 55, 47, 39, 31, 23, 15,
                                  7, 62, 54, 46, 38, 30, 22, 
                                 14,  6, 61, 53, 45, 37, 29, 
                                 21, 13,  5, 28, 20, 12,  4 };
    constexpr size_t pc2[48] = { 14, 17, 11, 24,  1,  5,  3, 28, 
                                 15,  6, 21, 10, 23, 19, 12,  4, 
                                 26,  8, 16,  7, 27, 20, 13,  2, 
                                 41, 52, 31, 37, 47, 55, 30, 40, 
                                 51, 45, 33, 48, 44, 49, 39, 56, 
                                 34, 53, 46, 42, 50, 36, 29, 32 }; 
    // left shift both half-keys 1 bit 
    auto key_shift = [](uint8_t key[])->void {
        bool a = getbit(key, 0);
        bool b = getbit(key, 28);
        for (size_t i = 0; i < 55; ++i)
            setbit(key, i, getbit(key, i + 1));
        setbit(key, 27, a);
        setbit(key, 55, b);
    };

    uint8_t key_p[7] = { 0 };

    for (size_t i = 0; i < 56; ++i)
        setbit(key_p, i, getbit(key, pc1[i] - 1));

    std::array<std::array<uint8_t, 6>, 16> subkeys;

    
    for (size_t i = 0; i < 16; ++i) {
        key_shift(key_p);
        if (i != 0 && i != 1 && i != 8 && i != 15) key_shift(key_p);
        for (size_t j = 0; j < 48; ++j)
            setbit(&subkeys[i][0], j, getbit(key_p, pc2[j] - 1));
    }
    
    
    return subkeys;
}

inline uint32_t f_function(const uint32_t rn_1, const std::array<uint8_t, 6>& key_n) {
    constexpr size_t E[] = { 32,  1,  2,  3,  4,  5, 
                              4,  5,  6,  7,  8,  9, 
                              8,  9, 10, 11, 12, 13, 
                             12, 13, 14, 15, 16, 17, 
                             16, 17, 18, 19, 20, 21, 
                             20, 21, 22, 23, 24, 25, 
                             24, 25, 26, 27, 28, 29, 
                             28, 29, 30, 31, 32,  1 }; 
    constexpr size_t S[8][64] = {
        { 14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7, 
           0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8, 
           4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0, 
          15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13 },
        { 15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10, 
           3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5, 
           0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15, 
          13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9 },
        { 10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8, 
          13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1, 
          13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7, 
           1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12 },
        {  7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15, 
          13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9, 
          10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4, 
           3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14 },
        {  2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9, 
          14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6, 
           4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14, 
          11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3 },
        { 12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11, 
          10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8, 
           9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6, 
           4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13 },
        {  4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1, 
          13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6, 
           1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2, 
           6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12},
        { 13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
           1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2, 
           7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8, 
           2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11 }
    };

    uint8_t e[6];
    for (size_t i = 0; i < 48; ++i)
        setbit(e, i, getbit(&rn_1, E[i] - 1));

    for (size_t i = 0; i < 6; ++i)
        e[i] ^= key_n[i];

    uint32_t val = 0;

    for (size_t i = 0; i < 8; ++i) {
        uint8_t Sbox_value = S[i][(getbit(e, 6 * i + 0)) << 5 |
                                  (getbit(e, 6 * i + 5)) << 4 |
                                  (getbit(e, 6 * i + 1)) << 3 |
                                  (getbit(e, 6 * i + 2)) << 2 |
                                  (getbit(e, 6 * i + 3)) << 1 |
                                  (getbit(e, 6 * i + 4)) << 0
                                 ];
        setbit(&val, i * 4 + 0, getbit(&Sbox_value, 4));
        setbit(&val, i * 4 + 1, getbit(&Sbox_value, 5));
        setbit(&val, i * 4 + 2, getbit(&Sbox_value, 6));
        setbit(&val, i * 4 + 3, getbit(&Sbox_value, 7));
    }

    constexpr size_t P[32] = { 16,  7, 20, 21, 29, 12, 28, 17, 
                                1, 15, 23, 26,  5, 18, 31, 10, 
                                2,  8, 24, 14, 32, 27,  3,  9,
                               19, 13, 30,  6, 22, 11,  4, 25 };
    uint32_t p_val;
    for (size_t i = 0; i < 32; ++i)
        setbit(&p_val, i, getbit(&val, P[i] - 1));

    return p_val;
}

// plain is 64 bits, cipher is 64 bits
void des_cfb_iteration(const uint8_t* plain, const std::array<std::array<uint8_t, 6>, 16>& key, uint8_t* cipher) {
    constexpr size_t IP[64] = { 58, 50, 42, 34, 26, 18, 10,  2, 
                                60, 52, 44, 36, 28, 20, 12,  4, 
                                62, 54, 46, 38, 30, 22, 14,  6, 
                                64, 56, 48, 40, 32, 24, 16,  8, 
                                57, 49, 41, 33, 25, 17,  9,  1, 
                                59, 51, 43, 35, 27, 19, 11,  3, 
                                61, 53, 45, 37, 29, 21, 13,  5, 
                                63, 55, 47, 39, 31, 23, 15,  7 };
    // plain text after ip
    uint8_t ip[8];
    for (size_t i = 0; i < 64; ++i)
        setbit(ip, i, getbit(plain, IP[i] - 1));

    uint32_t ln, rn, ln_1, rn_1;
    ln_1 = ip[0] << 0 | ip[1] << 8 | ip[2] << 16 | ip[3] << 24;
    rn_1 = ip[4] << 0 | ip[5] << 8 | ip[6] << 16 | ip[7] << 24;

    for (size_t i = 0; i < 16; ++i) {
        ln = rn_1;
        rn = ln_1 ^ f_function(rn_1, key[i]);

        ln_1 = ln;
        rn_1 = rn;
    }

    constexpr size_t IP_i[64] = { 40,  8, 48, 16, 56, 24, 64, 32, 
                                  39,  7, 47, 15, 55, 23, 63, 31, 
                                  38,  6, 46, 14, 54, 22, 62, 30, 
                                  37,  5, 45, 13, 53, 21, 61, 29, 
                                  36,  4, 44, 12, 52, 20, 60, 28, 
                                  35,  3, 43, 11, 51, 19, 59, 27, 
                                  34,  2, 42, 10, 50, 18, 58, 26, 
                                  33,  1, 41,  9, 49, 17, 57, 25 };
    uint64_t rnln = uint64_t(ln) << 32 | rn;
    uint64_t ip_i_rnln;
    for (size_t i = 0; i < 64; ++i)
        setbit(&ip_i_rnln, i, getbit(&rnln, IP_i[i] - 1));

    cipher[0] = ip_i_rnln >>  0;
    cipher[1] = ip_i_rnln >>  8;
    cipher[2] = ip_i_rnln >> 16;
    cipher[3] = ip_i_rnln >> 24;
    cipher[4] = ip_i_rnln >> 32;
    cipher[5] = ip_i_rnln >> 40;
    cipher[6] = ip_i_rnln >> 48;
    cipher[7] = ip_i_rnln >> 56;
}


void des_cfb(const void* plain, const size_t length, const void* key, const void* IV, void* cipher) {
    uint8_t* plain_ = (uint8_t*)plain;
    const uint8_t* key_ = (const uint8_t*)key;
    uint8_t* cipher_ = (uint8_t*)cipher;

    uint8_t buffer[8];
    memcpy(buffer, IV, 8);

    auto subkeys = generateSubkeys(key_);

    for (size_t i = 0; i < length; ++i) { 
        des_cfb_iteration(buffer, subkeys, cipher_ + i);
        cipher_[i] ^= plain_[i];
        for (size_t i = 0; i < 7; ++i)
            buffer[i] = buffer[i + 1];
        buffer[7] = cipher_[i];
    }

}

int main(int argc, char** argv) {
    if (argc == 1) return 0;

    std::ifstream fin(argv[1]);

    fin.seekg(0, std::ios::end);
    std::string buffer;
    buffer.reserve(fin.tellg());
    fin.seekg(0, std::ios::beg);


    buffer.assign((std::istreambuf_iterator<char>(fin)),
                   std::istreambuf_iterator<char>());

    fin.close();

    unsigned char key[8] = { 0 };
    unsigned char IV[8] = { 0 };

    if (argc == 3) {
        fin.open(argv[2]);
        fin.seekg(0, std::ios::beg);
        char buffer[3] = { 0 };
        for (size_t i = 0; i < 8; ++i) {
            fin.read(buffer, 2);
            key[i] = std::stoi(buffer, 0, 16);
        }
        fin.close();
    }


    std::vector<char> cipher(buffer.length() + 8, 0);
    des_cfb(buffer.data(), buffer.length(), key, IV, &cipher[0]);

    // for (size_t i = 0; i < buffer.length(); ++i)
    //     printf("%02x", int(cipher[i]) & 0xff);
    // printf("\n");
    
}

