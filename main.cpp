/*
 * ════════════════════════════════════════════════════════════════
 *  BigInt Library — Showcase Demo
 *  Demonstrates every major feature of the arbitrary-precision
 *  integer library with formatted, annotated output.
 * ════════════════════════════════════════════════════════════════
 */
#include "BigInt.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

// ── Helpers ────────────────────────────────────────────────────
static void banner(const std::string& title) {
    std::string bar(60, '=');
    std::cout << "\n" << bar << "\n";
    int pad = (60 - (int)title.size()) / 2;
    std::cout << std::string(pad, ' ') << title << "\n";
    std::cout << bar << "\n";
}
static void section(const std::string& s) {
    std::cout << "\n-- " << s << " ";
    if (s.size() < 54)
        std::cout << std::string(54 - s.size(), '-');
    std::cout << "\n";
}
static void show(const std::string& label, const BigInt& val) {
    std::string v = val.toString();
    if (v.size() > 60)
        std::cout << "  " << label << "\n    " << v.substr(0,30)
                  << "..." << v.substr(v.size()-10)
                  << "  (" << val.numDigits() << " digits)\n";
    else
        std::cout << "  " << std::left << std::setw(30) << label << val << "\n";
}

int main() {
    banner("BigInt  |  Arbitrary-Precision Integer Library");
    std::cout << "  Internal base : 10^9  (each limb stores 9 decimal digits)\n";
    std::cout << "  Multiplication: Karatsuba (threshold: 16 limbs = 144 decimal digits)\n";

    // ────────────────────────────────────────────────────────────
    section("1. Basic Construction");
    // ────────────────────────────────────────────────────────────
    BigInt a(123456789012345LL);
    BigInt b("-9999999999999999999999999999999");
    BigInt c("314159265358979323846264338327950288");

    show("from long long  : ", a);
    show("from string(-) : ", b);
    show("pi digits       : ", c);

    // ────────────────────────────────────────────────────────────
    section("2. Arithmetic on Large Numbers");
    // ────────────────────────────────────────────────────────────
    BigInt x("123456789012345678901234567890");
    BigInt y("987654321098765432109876543210");

    show("x               : ", x);
    show("y               : ", y);
    show("x + y           : ", x + y);
    show("y - x           : ", y - x);
    show("x * y           : ", x * y);   // 30-digit × 30-digit = 60-digit
    show("y / x           : ", y / x);
    show("y % x           : ", y % x);

    // ────────────────────────────────────────────────────────────
    section("3. Karatsuba Multiplication – 100-digit numbers");
    // ────────────────────────────────────────────────────────────
    BigInt big1("1" + std::string(99, '0'));   // 10^99
    BigInt big2("9" + std::string(99, '9'));   // 10^100 - 1

    auto t0 = std::chrono::high_resolution_clock::now();
    BigInt product = big1 * big2;
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    show("big1 * big2     : ", product);
    std::cout << "  Time (Karatsuba)    : " << ms << " ms\n";
    std::cout << "  Result digits       : " << product.numDigits() << "\n";

    // ────────────────────────────────────────────────────────────
    section("4. Power  (2^256 — important in cryptography)");
    // ────────────────────────────────────────────────────────────
    BigInt two(2);
    BigInt p256 = two.pow(256);
    show("2^256           : ", p256);
    std::cout << "  Digits              : " << p256.numDigits() << "\n";

    // ────────────────────────────────────────────────────────────
    section("5. Factorial  –  100! and 1000!");
    // ────────────────────────────────────────────────────────────
    BigInt f100  = BigInt::factorial(100);
    BigInt f1000 = BigInt::factorial(1000);
    show("100!            : ", f100);
    show("1000!           : ", f1000);
    std::cout << "  Digits in 100!  : " << f100.numDigits()  << "\n";
    std::cout << "  Digits in 1000! : " << f1000.numDigits() << "\n";

    // ────────────────────────────────────────────────────────────
    section("6. Fibonacci – F(1000) and F(10000)");
    // ────────────────────────────────────────────────────────────
    BigInt fib1000  = BigInt::fibonacci(1000);
    BigInt fib10000 = BigInt::fibonacci(10000);
    show("F(1000)         : ", fib1000);
    show("F(10000)        : ", fib10000);
    std::cout << "  Digits in F(1000)  : " << fib1000.numDigits()  << "\n";
    std::cout << "  Digits in F(10000) : " << fib10000.numDigits() << "\n";

    // ────────────────────────────────────────────────────────────
    section("7. Integer Square Root");
    // ────────────────────────────────────────────────────────────
    BigInt n1("123456789012345678901234567890");
    BigInt sq = n1.isqrt();
    show("N               : ", n1);
    show("floor(sqrt(N))  : ", sq);
    // Verify: sq^2 <= N < (sq+1)^2
    bool ok = (sq * sq <= n1) && (n1 < (sq + BigInt(1)) * (sq + BigInt(1)));
    std::cout << "  Verification (sq^2 <= N < (sq+1)^2): " << (ok ? "PASS" : "FAIL") << "\n";

    // ────────────────────────────────────────────────────────────
    section("8. GCD and LCM");
    // ────────────────────────────────────────────────────────────
    BigInt u("246813579246813579246813579");
    BigInt v("135792468135792468135792468");
    show("u               : ", u);
    show("v               : ", v);
    show("gcd(u, v)       : ", u.gcd(v));
    show("lcm(u, v)       : ", u.lcm(v));

    // ────────────────────────────────────────────────────────────
    section("9. Modular Exponentiation  (simulated RSA core)");
    // ────────────────────────────────────────────────────────────
    BigInt modulus ("100000000000000000000000000000000000000000000003");
    BigInt base_rsa("12345678901234567890123456789");
    BigInt exp_e   ("65537");   // common RSA public exponent

    BigInt cipher  = base_rsa.modPow(exp_e,   modulus);

    show("message         : ", base_rsa);
    show("ciphertext      : ", cipher);
    std::cout << "  (modPow with 45-digit modulus and e=65537)\n";

    // ────────────────────────────────────────────────────────────
    section("10. Modular Inverse");
    // ────────────────────────────────────────────────────────────
    BigInt num_inv("123456789");
    BigInt mod_inv("1000000007");   // prime
    BigInt inv = num_inv.modInverse(mod_inv);
    show("a               : ", num_inv);
    show("mod             : ", mod_inv);
    show("a^-1 mod p      : ", inv);
    show("a * a^-1 mod p  : ", (num_inv * inv) % mod_inv);   // should be 1

    // ────────────────────────────────────────────────────────────
    section("11. Comparison Operators");
    // ────────────────────────────────────────────────────────────
    BigInt P("999999999999999999999");
    BigInt Q("1000000000000000000000");
    std::cout << "  P = " << P << "\n";
    std::cout << "  Q = " << Q << "\n";
    std::cout << "  P < Q  : " << std::boolalpha << (P < Q)  << "\n";
    std::cout << "  P > Q  : " << (P > Q)  << "\n";
    std::cout << "  P == Q : " << (P == Q) << "\n";
    std::cout << "  P != Q : " << (P != Q) << "\n";

    // ────────────────────────────────────────────────────────────
    section("12. Negative Numbers & Unary Operators");
    // ────────────────────────────────────────────────────────────
    BigInt neg("-98765432109876543210");
    show("neg             : ", neg);
    show("-neg            : ", -neg);
    show("neg.abs()       : ", neg.abs());
    show("neg + 1         : ", neg + BigInt(1));
    show("neg - 1         : ", neg - BigInt(1));
    show("neg * 2         : ", neg * BigInt(2));
    show("neg / 3         : ", neg / BigInt(3));

    // ────────────────────────────────────────────────────────────
    section("13. Operator Overloading — Fluent Syntax");
    // ────────────────────────────────────────────────────────────
    BigInt z = BigInt(2).pow(64) - BigInt(1);   // 2^64 - 1  (max uint64)
    std::cout << "  2^64 - 1 = " << z << "\n";
    std::cout << "  is even  = " << std::boolalpha << z.isEven() << "\n";
    std::cout << "  is odd   = " << z.isOdd()  << "\n";
    std::cout << "  sign     = " << z.sign()   << "\n";
    std::cout << "  digits   = " << z.numDigits() << "\n";

    BigInt counter(0LL);
    for (int i = 0; i < 5; ++i) ++counter;
    std::cout << "  counter (5 prefix ++) : " << counter << "\n";

    // ────────────────────────────────────────────────────────────
    section("14. Number Theory: Primality & Prime Generation");
    // ────────────────────────────────────────────────────────────
    BigInt prime_cand1("1000000007"); // Prime
    BigInt prime_cand2("1000000011"); // Composite (1000000011 = 3 * 333333337)
    std::cout << "  isPrime(" << prime_cand1 << ") : " << std::boolalpha << prime_cand1.isPrime() << "\n";
    std::cout << "  isPrime(" << prime_cand2 << ") : " << prime_cand2.isPrime() << "\n";
    
    BigInt next_p = prime_cand1.nextPrime();
    std::cout << "  nextPrime(" << prime_cand1 << ") : " << next_p << "\n";

    BigInt millionth_prime = BigInt::nthPrime(100);
    std::cout << "  100-th prime                : " << millionth_prime << "\n";

    // ────────────────────────────────────────────────────────────
    section("15. Advanced Math: k-th Root, Binomial Coefficient & Trailing Zeros");
    // ────────────────────────────────────────────────────────────
    BigInt cube_val("1000000000000000000000000000"); // 10^27
    BigInt cbrt_res = cube_val.nthRoot(3);
    show("cube_val (10^27)          : ", cube_val);
    show("floor(cbrt(cube_val))     : ", cbrt_res);

    BigInt binom = BigInt::binomial(100, 10); // C(100, 10)
    std::cout << "  binomial(100, 10)           : " << binom << "\n";

    BigInt fact_100 = BigInt::factorial(100);
    std::cout << "  100! trailing decimal zeros : " << fact_100.trailingZeros() << "\n";

    BigInt huge_val = BigInt(2).pow(1000);
    std::cout << "  approxLog10(2^1000)         : " << huge_val.approxLog10() << "\n";
    std::cout << "  approxLog2(2^1000)          : " << huge_val.approxLog2() << "\n";

    // ────────────────────────────────────────────────────────────
    section("16. Bitwise Operations & Shifts");
    // ────────────────────────────────────────────────────────────
    BigInt b_val1(12); // 1100
    BigInt b_val2(10); // 1010
    std::cout << "  12 & 10  = " << (b_val1 & b_val2) << "   (AND)\n";
    std::cout << "  12 | 10  = " << (b_val1 | b_val2) << "  (OR)\n";
    std::cout << "  12 ^ 10  = " << (b_val1 ^ b_val2) << "   (XOR)\n";

    BigInt shift_val(1);
    std::cout << "  1 << 10  = " << (shift_val << 10) << "  (Bit shift left)\n";
    BigInt shift_right(1024);
    std::cout << "  1024 >> 5 = " << (shift_right >> 5) << "   (Bit shift right)\n";

    BigInt inspect_val(42); // 101010
    std::cout << "  42 bit length               : " << inspect_val.bitLength() << "\n";
    std::cout << "  42 testBit(3)               : " << std::boolalpha << inspect_val.testBit(3) << " (10[1]010)\n";
    std::cout << "  42 setBit(0)                : " << inspect_val.setBit(0) << " (101011 = 43)\n";

    // ────────────────────────────────────────────────────────────
    section("17. Base Conversions (Binary / Hex / String)");
    // ────────────────────────────────────────────────────────────
    BigInt conv_num("255");
    std::cout << "  255 to hex                  : " << conv_num.toHex() << "\n";
    std::cout << "  255 to binary               : " << conv_num.toBinary() << "\n";
    
    BigInt from_hex = BigInt::fromHex("deadbeef");
    std::cout << "  deadbeef from hex           : " << from_hex << "\n";

    BigInt from_bin = BigInt::fromBinary("110101");
    std::cout << "  110101 from binary          : " << from_bin << "\n";

    // ────────────────────────────────────────────────────────────
    section("18. Public divmod (Quotient & Remainder simultaneously)");
    // ────────────────────────────────────────────────────────────
    BigInt div_num("1000000000000000000000000000001");
    BigInt div_den("3");
    std::pair<BigInt, BigInt> res_div = div_num.divmod(div_den);
    show("div_num                     : ", div_num);
    show("quotient                    : ", res_div.first);
    show("remainder                   : ", res_div.second);

    banner("All demos complete!");
    return 0;
}
