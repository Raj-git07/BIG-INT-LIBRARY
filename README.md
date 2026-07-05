# BigInt — Arbitrary-Precision Integer Library (C++17)

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build & Test Status](https://img.shields.io/badge/tests-248%20%2F%20248%20passing-brightgreen.svg)](#)

A high-performance, arbitrary-precision (infinite-precision) integer arithmetic library written in modern C++17. Designed with a base-$10^9$ representation (storing 9 decimal digits per limb) for optimal memory layout and calculation speeds.

---

## 🚀 Key Features

*   **Standard Arithmetic**: Supports infinite-precision addition (`+`), subtraction (`-`), multiplication (`*`), division (`/`), and modulo (`%`).
*   **Fast Multiplication**: Automatically switches to the $O(n^{1.585})$ **Karatsuba Algorithm** for large inputs (threshold: 16 limbs / 144 digits).
*   **Number Theory Utilities**:
    *   Probabilistic **Miller-Rabin Primality Test** with deterministic witnesses for numbers $< 3.3 \times 10^{24}$ (ensures exact primality for 64-bit bounds and beyond).
    *   Prime generators: `nextPrime()` and `nthPrime()`.
    *   Modular Arithmetic: Fast exponentiation (`modPow`) and modular inverse (`modInverse`).
    *   Greatest Common Divisor (`gcd`) and Least Common Multiple (`lcm`).
*   **Advanced Mathematics**:
    *   **k-th Root**: Generalized Newton-Raphson approximation `nthRoot(k)` and optimized integer square root `isqrt()`.
    *   **Binomial Coefficient**: $C(n, k)$ calculation using multiplicative formula to prevent overflow.
    *   **Approximations**: `approxLog2()` and `approxLog10()` calculations returning standard `double`.
    *   **Trailing Zeros**: Fast decimal trailing zeros count.
*   **Bitwise & Manipulation**:
    *   Bit shifts (`<<`, `>>`) and bitwise logical gates (`&`, `|`, `^`).
    *   Bit inspection & manipulation: `bitLength()`, `testBit()`, `setBit()`, and `clearBit()`.
*   **Base Conversions**: Convert dynamically to/from Hexadecimal and Binary string formats.
*   **Integration**: Complete operator overloading, fluent syntax support, stream I/O (`std::ostream` / `std::istream`), and structured helper `divmod()`.

---

## 🛠️ Project Structure

```
├── BigInt.h          # Header declaring the BigInt class API
├── BigInt.cpp        # Full implementation source file
├── main.cpp          # Interactive demo showcase of library features
├── test.cpp          # Comprehensive unit test suite (248 tests)
├── Makefile          # Gnu Make build scripts
└── README.md         # Documentation
```

---

## ⚡ Getting Started & Compilation

You only need a modern C++ compiler supporting the **C++17** standard (e.g. GCC/g++).

### Build Demo & Test Suite

Using **Make**:
```bash
make          # Compiles both bigint_demo.exe and bigint_test.exe
```

Using **g++** directly:
```bash
# Compile Demo
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -o bigint_demo.exe BigInt.cpp main.cpp

# Compile Tests
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -o bigint_test.exe BigInt.cpp test.cpp
```

### Run Executables

Run the comprehensive unit test suite:
```bash
./bigint_test.exe
```

Run the showcase demo:
```bash
./bigint_demo.exe
```

---

## 💻 Usage Example

Here is a quick look at how easy it is to use the `BigInt` library:

```cpp
#include "BigInt.h"
#include <iostream>

int main() {
    // 1. Basic Construction
    BigInt a("123456789012345678901234567890");
    BigInt b(9876543210LL);

    // 2. Standard Arithmetic
    BigInt sum = a + b;
    BigInt product = a * b;
    std::cout << "Sum: " << sum << "\n";
    std::cout << "Product: " << product << "\n";

    // 3. Number Theory & Advanced Math
    BigInt p("1000000007"); // Prime
    if (p.isPrime()) {
        std::cout << p << " is prime!\n";
    }

    BigInt power = BigInt(2).pow(256);
    std::cout << "2^256 has " << power.numDigits() << " digits.\n";

    // 4. Bit shifts and base conversions
    BigInt shifted = BigInt(1) << 10; // 1024
    std::cout << "Binary: " << shifted.toBinary() << "\n"; // 10000000000
    std::cout << "Hex: " << BigInt("255").toHex() << "\n"; // ff

    return 0;
}
```

---

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.
