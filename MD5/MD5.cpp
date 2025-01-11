/* 
 * MD5.cpp
 *
 * Implementation of the MD5 hash function. This program takes a message as input
 * and outputs the MD5 hash value of the message. The message can be given as a
 * string in the command line, or as a text file. The hash value can be written
 * to a file or to the console.
 * 
 */

#include "../io.h"
#include <iostream>
#include <cassert>

/* 
 * A table of constants used in the MD5 algorithm. These constants are used in
 * the main loop of the algorithm to update the state of the hash function.
 */
static constexpr uint32_t K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

/* 
 * A table of shift amounts used in the MD5 algorithm. These shift amounts are
 * used in the main loop of the algorithm to update the state of the hash function.
 */
static constexpr int S[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

/* 
 * A usage string to be displayed if the user provides incorrect arguments.
 */
static const std::string usage = "\nUsage:\nMD5 --message=\"...\" [--outputFile="
    "\"...\"]\nOR\n" "MD5 --messageFile=\"...\" [--outputFile=\"...\"]\n";

/*
 * The path to the output file, if provided by the user.
 */
static std::string out_file;

/*
 * Retrieve the message to be hashed from the command line arguments. The message
 * can be provided as a string or as a file. If the message is provided as a file,
 * the contents of the file are read into a vector of bytes.
 */
static std::vector<uint8_t> md5_get_message(int argc, char** argv) {
    std::vector<std::string> accepted_args = {"message", "messageFile", "outputFile"};
    std::map<std::string, std::string> args = parse_args(argc, argv, accepted_args, usage);
    if (args.find("outputFile") != args.end()) {
        out_file = args["outputFile"];
    }
    if (args.find("message") == args.end()) {
        if (args.find("messageFile") == args.end()) {
            std::cerr << "Error: No message provided." << std::endl;
            std::cerr << usage << std::endl;
            exit(1);
        }
        return read_file_bytes(args["messageFile"]);
    }
    if (args.find("messageFile") != args.end()) {
        std::cerr << "Error: Both message and messageFile provided." << std::endl;
        std::cerr << usage << std::endl;
        exit(1);
    }
    return std::vector<uint8_t>(args["message"].begin(), args["message"].end());
}

/* 
 * Given a message, pad it to a multiple of 64 bytes (512 bits) as per the MD5
 * specification. The padding consists of a 1 bit, followed by 0 bits until
 * there are 8 bytes remaining (until the message is padded to a multiple of 64
 * bytes). Finally, the original message length is appended as a 64-bit integer
 * in little-endian format.
 */
static void md5_pad_message(std::vector<uint8_t>& message) {
    uint64_t originalLength = message.size() * 8;
    message.push_back(0x80);
    while ((message.size() + 8) % 64 != 0) {
        message.push_back(0);
    }
    for (int i = 0; i < 8; ++i) {
        message.push_back((originalLength >> (i * 8)) & 0xff);
    }
}

/*
 * Given a message and an index i, return the i-th 512-bit chunk of the message as a
 * vector of 16 32-bit integers.
 */
static std::vector<uint32_t> md5_get_chunk(const std::vector<uint8_t>& message, int i) {
    assert(i >= 0 && i < (int) message.size());
    std::vector<uint32_t> chunk(16);
    for (int j = 0; j < 16; ++j) {
        for (int k = 0; k < 4; ++k) {
            chunk[j] |= message[i + j * 4 + k] << (8 * k);
        }
    }
    return chunk;
}

/* 
 * Rotate a 32-bit integer left by n bits.
 */
static uint32_t md5_rotate(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

/*
 * Process a 512-bit chunk of the message using the MD5 algorithm. The chunk is
 * processed in 16 rounds, with each round updating the state of the hash function.
 */
static std::vector<uint32_t> md5_process_chunk(const std::vector<uint32_t>& chunk,
    uint32_t A, uint32_t B, uint32_t C, uint32_t D) {
    for (int i = 0; i < 64; ++i) {
        uint32_t input_word, fghi;
        if (i < 16) {
            input_word = chunk[i];
            fghi = (B & C) | ((~B) & D);
        } else if (i < 32) {
            input_word = chunk[(5 * i + 1) % 16];
            fghi = (D & B) | ((~D) & C);
        } else if (i < 48) {
            input_word = chunk[(3 * i + 5) % 16];
            fghi = B ^ C ^ D;
        } else {
            input_word = chunk[(7 * i) % 16];
            fghi = C ^ (B | (~D));
        }
        uint32_t temp = D;
        D = C;
        C = B;
        B = md5_rotate(fghi + A + input_word + K[i], S[i]) + B;
        A = temp;
    }
    return {A, B, C, D};
}


int main(int argc, char** argv) {
    std::vector<uint8_t> message = md5_get_message(argc, argv);
    md5_pad_message(message);

    // Initial MD5 state is a hard-coded constant split into four 32-bit words.
    uint32_t A = 0x67452301;
    uint32_t B = 0xefcdab89;
    uint32_t C = 0x98badcfe;
    uint32_t D = 0x10325476;

    // Process each 512-bit chunk of the message in sequence
    for (int i = 0; i < (int) message.size(); i += 64) {
        std::vector<uint32_t> chunk = md5_get_chunk(message, i);
        std::vector<uint32_t> result = md5_process_chunk(chunk, A, B, C, D);
        A += result[0];
        B += result[1];
        C += result[2];
        D += result[3];
    }

    // Convert the final state to bytes to ensure correct endianness
    std::vector<uint8_t> result_bytes(16);
    for (int i = 0; i < 4; ++i) {
        result_bytes[i] = (A >> (i * 8)) & 0xff;
        result_bytes[i + 4] = (B >> (i * 8)) & 0xff;
        result_bytes[i + 8] = (C >> (i * 8)) & 0xff;
        result_bytes[i + 12] = (D >> (i * 8)) & 0xff;
    }

    if (out_file.empty()) {
        std::cout << "hash: " << to_hex_string(result_bytes) << std::endl;
    } else {
        std::cout << "Writing hash to " << out_file << "...";
        write_file(out_file, to_hex_string(result_bytes));
        std::cout << " Done." << std::endl;
    }
}
