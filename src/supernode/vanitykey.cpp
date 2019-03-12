#include "crypto/crypto.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

template <size_t N> std::string hexify(char *data) {
    std::ostringstream s;
    s << std::hex << std::setw(2);
    for (size_t i = 0; i < N; i++)
        s << std::setw(2) << std::setfill('0') << (size_t) reinterpret_cast<unsigned char *>(data)[i];
    return s.str();
}

int main(int argc, const char** argv)
{
    const std::string bad = "Error: usage: " + std::string(argv[0]) + " vanitykey HEXPREFIX -- where HEXPREFIX is some hex prefix like ab49c (min length 2).  Be warned: each additional letter takes 16 times as long to find!\n\n";
    if (argc != 2) {
        std::cerr << bad;
        return 1;
    }
    std::string prefix(argv[1]);
    if (prefix.size() < 2) {
        std::cerr << bad;
        return 1;
    }
    for (char c : prefix)
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            std::cerr << bad;
            return 1;
        }
    const char first = ((prefix[0] <= '9' ? prefix[0] - '0' : prefix[0] - ('a'-10)) << 4) +
        (prefix[1] <= '9' ? prefix[1] - '0' : prefix[1] - ('a'-10));

    long expect = 1 << (4 * prefix.size());
    std::cout << "Generating key pairs; this is going to take, on average, " << expect << " attempts\n";

    size_t i = 0;
    while (true) {
        crypto::public_key pubkey;
        crypto::secret_key privkey;
        crypto::generate_keys(pubkey, privkey);

        int first_byte = *reinterpret_cast<unsigned char *>(pubkey.data);

        if (pubkey.data[0] == first) {
            std::string pub_str = hexify<32>(pubkey.data);
            if (std::equal(prefix.begin(), prefix.end(), pub_str.begin())) {
                std::string priv_str = hexify<32>(privkey.data);
                std::ofstream f(pub_str);
                f << priv_str;
                std::cout << "Found a match on try " << i << " (" << (i * 100 / expect) << "% effort).\n";
                std::cout << "Saved the priv key for pub key: " << pub_str << " into a file of the same name.\n";
                return 0;
            }
        }
        if (++i % 20000 == 0) {
            std::cout << "Generated " << i << " keys (" << (i*100/expect) << "% effort), still searching\n";
        }
    }
}
