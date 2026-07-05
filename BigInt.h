/*
 * ============================================================
 *  BigInt — Arbitrary-Precision Integer Library  (C++17)
 * ============================================================
 *  Features:
 *    • Unlimited precision  (values beyond native 64-bit limits)
 *    • Karatsuba Multiplication  – O(n^1.585)
 *    • Long Division with remainder  – O(n^2 log B)
 *    • Modular Arithmetic: modPow, modInverse, GCD, LCM
 *    • Operator Overloading for intuitive arithmetic syntax
 *    • Integer Square Root (Newton's method)
 *    • Power, Factorial, Fibonacci utilities
 *    • Miller-Rabin Primality Test + Prime Generation
 *    • k-th Root (Newton's method generalization)
 *    • Binomial Coefficient  C(n, k)
 *    • Trailing Zero Count, Approximate log2/log10
 *    • Bit Shifts (<<, >>), Bitwise AND / OR / XOR
 *    • Bit-level inspection: bitLength, testBit, setBit, clearBit
 *    • Base Conversion: toHex, toBinary, fromHex, fromBinary
 *    • Public divmod for simultaneous quotient + remainder
 *
 *  Internal representation:
 *    Digits stored in base 10^9 (little-endian), signed magnitude.
 * ============================================================
 */
#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <stdexcept>

class BigInt {
public:
    // ─────────────────────── Constructors ────────────────────────
    BigInt();                           // 0
    BigInt(int n);                      // from int literal  (avoids ambiguity)
    BigInt(long long n);                // from native integer
    BigInt(const std::string& s);       // from decimal string  (e.g. "12345678901234567890")
    BigInt(const char* s);              // from C-string literal

    BigInt(const BigInt&)            = default;
    BigInt(BigInt&&)                 = default;
    BigInt& operator=(const BigInt&) = default;
    BigInt& operator=(BigInt&&)      = default;
    BigInt& operator=(long long n);
    BigInt& operator=(const std::string& s);

    // ─────────────────────── Arithmetic ──────────────────────────
    BigInt  operator+ (const BigInt& rhs) const;
    BigInt  operator- (const BigInt& rhs) const;
    BigInt  operator* (const BigInt& rhs) const;
    BigInt  operator/ (const BigInt& rhs) const;
    BigInt  operator% (const BigInt& rhs) const;

    BigInt& operator+=(const BigInt& rhs);
    BigInt& operator-=(const BigInt& rhs);
    BigInt& operator*=(const BigInt& rhs);
    BigInt& operator/=(const BigInt& rhs);
    BigInt& operator%=(const BigInt& rhs);

    // ────────────────────── Unary / Inc-Dec ──────────────────────
    BigInt  operator-() const;
    BigInt  operator+() const;
    BigInt& operator++();           // prefix
    BigInt  operator++(int);        // postfix
    BigInt& operator--();
    BigInt  operator--(int);

    // ─────────────────────── Comparison ──────────────────────────
    bool operator==(const BigInt& rhs) const;
    bool operator!=(const BigInt& rhs) const;
    bool operator< (const BigInt& rhs) const;
    bool operator<=(const BigInt& rhs) const;
    bool operator> (const BigInt& rhs) const;
    bool operator>=(const BigInt& rhs) const;

    // ───────────────────────── I / O ─────────────────────────────
    friend std::ostream& operator<<(std::ostream& os, const BigInt& n);
    friend std::istream& operator>>(std::istream& is,       BigInt& n);

    // ─────────────────────── Utilities ───────────────────────────
    std::string toString()   const;
    bool        isZero()     const;
    bool        isOne()      const;
    bool        isNegative() const;
    bool        isPositive() const;
    bool        isEven()     const;
    bool        isOdd()      const;
    BigInt      abs()        const;
    int         sign()       const;         // -1 | 0 | 1
    int         numDigits()  const;         // decimal digit count

    // ─────────────────── Mathematical Functions ──────────────────
    BigInt pow      (long long exp)                          const;  // this ^ exp
    BigInt modPow   (const BigInt& exp, const BigInt& mod)   const;  // this^exp mod m
    BigInt gcd      (const BigInt& other)                    const;  // Euclidean GCD
    BigInt lcm      (const BigInt& other)                    const;  // LCM
    BigInt modInverse(const BigInt& mod)                     const;  // Extended-Euclidean inverse
    BigInt isqrt    ()                                       const;  // Floor of square root
    BigInt nthRoot  (long long k)                            const;  // Floor of k-th root (k >= 1)

    // Public divmod: returns {quotient, remainder} in one call
    std::pair<BigInt, BigInt> divmod(const BigInt& other) const;

    // ──────────────────── Static Utilities ───────────────────────
    static BigInt factorial(long long n);
    static BigInt fibonacci(long long n);
    static BigInt binomial (long long n, long long k);       // C(n, k) = n! / (k! * (n-k)!)
    static BigInt fromHex  (const std::string& hex);         // construct from hex string
    static BigInt fromBinary(const std::string& bin);        // construct from binary string

    // ──────────────────── Number Theory ──────────────────────────
    bool   isPrime  (int rounds = 20)  const;  // Miller-Rabin probabilistic test
    BigInt nextPrime()                 const;  // smallest prime strictly greater than *this
    static BigInt nthPrime(long long n);       // n-th prime (1-indexed, n >= 1)

    // ──────────────────── Approximations ─────────────────────────
    double approxLog2()  const;   // approximate log base-2  (double precision)
    double approxLog10() const;   // approximate log base-10 (double precision)

    // ──────────────────── Trailing Zeros ─────────────────────────
    long long trailingZeros() const;   // count trailing decimal zeros

    // ──────────────────── Bit Operations ─────────────────────────
    BigInt  operator<<(long long shift) const;   // left bit-shift  (multiply by 2^shift)
    BigInt  operator>>(long long shift) const;   // right bit-shift (floor divide by 2^shift)
    BigInt& operator<<=(long long shift);
    BigInt& operator>>=(long long shift);

    BigInt  operator&(const BigInt& rhs) const;  // bitwise AND
    BigInt  operator|(const BigInt& rhs) const;  // bitwise OR
    BigInt  operator^(const BigInt& rhs) const;  // bitwise XOR
    BigInt& operator&=(const BigInt& rhs);
    BigInt& operator|=(const BigInt& rhs);
    BigInt& operator^=(const BigInt& rhs);

    long long bitLength()              const;   // number of bits to represent |this|
    bool      testBit  (long long k)   const;   // test k-th bit (0 = LSB)
    BigInt    setBit   (long long k)   const;   // return copy with k-th bit set
    BigInt    clearBit (long long k)   const;   // return copy with k-th bit cleared

    // ──────────────────── Base Conversion ────────────────────────
    std::string toHex()    const;   // lowercase hex string (e.g. "ff1a2b")
    std::string toBinary() const;   // binary string (e.g. "101011")

private:
    // ── Internal representation ──────────────────────────────────
    static constexpr long long BASE          = 1'000'000'000LL; // 10^9
    static constexpr int       BASE_DIGITS   = 9;
    static constexpr int       KARA_THRESH   = 16; // Karatsuba kicks in above this many limbs

    std::vector<long long> mag;         // little-endian limbs in base 10^9
    bool                   negative = false;

    // ── Private helpers ──────────────────────────────────────────
    void   trim();
    static int   compareMag(const BigInt& a, const BigInt& b);
    static BigInt addMag   (const BigInt& a, const BigInt& b);
    static BigInt subMag   (const BigInt& a, const BigInt& b); // requires |a| >= |b|
    static BigInt mulScalar(const BigInt& a, long long s);     // s in [0, BASE)
    static BigInt naiveMul (const BigInt& a, const BigInt& b);
    static BigInt karatsubaMul(const BigInt& a, const BigInt& b);
    static BigInt shiftLimbs  (const BigInt& a, int k);        // a * BASE^k
    static std::pair<BigInt, BigInt> divmodMag(const BigInt& a, const BigInt& b);

    // Bit helpers: extract/build from binary limbs (base-2 representation)
    std::vector<bool> toBits() const;               // LSB first
    static BigInt fromBits(const std::vector<bool>& bits); // LSB first

    // Miller-Rabin witness test
    bool millerRabinTest(const BigInt& witness) const;
};
