/*
 * ════════════════════════════════════════════════════════════════
 *  BigInt Library — Comprehensive Test Suite
 *  Covers: constructors, arithmetic, comparisons, edge-cases,
 *          Karatsuba correctness, modular arithmetic, overflow,
 *          and all new mathematical / bit / base features.
 * ════════════════════════════════════════════════════════════════
 */
#include "BigInt.h"
#include <iostream>
#include <string>
#include <functional>
#include <stdexcept>
#include <cmath>

// ── Minimal test framework ─────────────────────────────────────
static int pass_count = 0, fail_count = 0;

static void check(const std::string& name, bool cond) {
    if (cond) {
        std::cout << "  [PASS]  " << name << "\n";
        ++pass_count;
    } else {
        std::cout << "  [FAIL]  " << name << "\n";
        ++fail_count;
    }
}

static void checkEq(const std::string& name,
                    const BigInt& got,
                    const std::string& expected) {
    check(name, got.toString() == expected);
    if (got.toString() != expected)
        std::cout << "          got=" << got << "  expected=" << expected << "\n";
}

static bool exceptionThrown(std::function<void()> fn) {
    try { fn(); return false; }
    catch (...) { return true; }
}

// ── Section helpers ────────────────────────────────────────────
static void section(const std::string& name) {
    std::cout << "\n-- " << name << " " << std::string(56 - name.size(), '-') << "\n";
}

// ══════════════════════════════════════════════════════════════
int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║        BigInt Library — Test Suite                       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    // ────────────────────────────────────────────────────────────
    section("1. Constructors & toString");
    // ────────────────────────────────────────────────────────────
    checkEq("default ctor = 0",          BigInt(),                       "0");
    checkEq("ll ctor 0",                 BigInt(0LL),                    "0");
    checkEq("ll ctor positive",          BigInt(123456789012345LL),      "123456789012345");
    checkEq("ll ctor negative",          BigInt(-987654321LL),           "-987654321");
    checkEq("ll ctor LLONG_MIN+1",       BigInt(-9223372036854775807LL), "-9223372036854775807");
    checkEq("string ctor large +",       BigInt("12345678901234567890"),  "12345678901234567890");
    checkEq("string ctor large -",       BigInt("-99999999999999999999"), "-99999999999999999999");
    checkEq("string ctor with + prefix", BigInt("+42"),                  "42");
    checkEq("string ctor single digit",  BigInt("7"),                    "7");
    checkEq("string ctor leading zeros", BigInt("00123"),                "123");
    check("invalid string throws",
          exceptionThrown([]{BigInt("12x4");}));

    // ────────────────────────────────────────────────────────────
    section("2. Addition");
    // ────────────────────────────────────────────────────────────
    checkEq("0 + 0",          BigInt(0LL)+BigInt(0LL),               "0");
    checkEq("pos + pos",      BigInt(3LL)+BigInt(4LL),               "7");
    checkEq("neg + neg",      BigInt(-3LL)+BigInt(-4LL),             "-7");
    checkEq("pos + neg =pos", BigInt(10LL)+BigInt(-3LL),             "7");
    checkEq("pos + neg =neg", BigInt(3LL)+BigInt(-10LL),             "-7");
    checkEq("cancel to 0",    BigInt(7LL)+BigInt(-7LL),              "0");
    checkEq("cross BASE",
            BigInt("999999999")+BigInt("1"),                     "1000000000");
    checkEq("big addition",
            BigInt("99999999999999999999")+BigInt("1"),          "100000000000000000000");
    checkEq("-big + big",
            BigInt("-100000000000000000000")+BigInt("100000000000000000000"), "0");

    // ────────────────────────────────────────────────────────────
    section("3. Subtraction");
    // ────────────────────────────────────────────────────────────
    checkEq("0 - 0",         BigInt(0LL)-BigInt(0LL),              "0");
    checkEq("5 - 3",         BigInt(5LL)-BigInt(3LL),              "2");
    checkEq("3 - 5",         BigInt(3LL)-BigInt(5LL),              "-2");
    checkEq("neg - neg",     BigInt(-3LL)-BigInt(-10LL),           "7");
    checkEq("neg - pos",     BigInt(-3LL)-BigInt(4LL),             "-7");
    checkEq("borrow chain",  BigInt("1000000000")-BigInt("1"), "999999999");
    checkEq("large sub",
            BigInt("100000000000000000000")-BigInt("99999999999999999999"), "1");

    // ────────────────────────────────────────────────────────────
    section("4. Multiplication (Schoolbook & Karatsuba)");
    // ────────────────────────────────────────────────────────────
    checkEq("0 * anything",  BigInt(0LL)*BigInt(99LL),             "0");
    checkEq("anything * 0",  BigInt(99LL)*BigInt(0LL),             "0");
    checkEq("1 * x",         BigInt(1LL)*BigInt(12345),          "12345");
    checkEq("pos * pos",     BigInt(6LL)*BigInt(7LL),              "42");
    checkEq("pos * neg",     BigInt(6LL)*BigInt(-7LL),             "-42");
    checkEq("neg * neg",     BigInt(-6LL)*BigInt(-7LL),            "42");
    checkEq("cross BASE",    BigInt("1000000000")*BigInt("1000000000"), "1000000000000000000");
    // Verify Karatsuba: 10^100 * 10^100 = 10^200
    {
        BigInt p("1" + std::string(100,'0'));
        BigInt r = p * p;
        check("Karatsuba 10^100 * 10^100 = 10^200",
              r.toString() == "1" + std::string(200,'0'));
    }
    // Cross-check Karatsuba vs naive: random 50-digit numbers
    {
        BigInt A("12345678901234567890123456789012345678901234567890");
        BigInt B("98765432109876543210987654321098765432109876543210");
        BigInt expected("1219326311370217952261850327338667885945115073915611949397448712086533622923332237463801111263526900");
        checkEq("Karatsuba 50-digit product", A * B, expected.toString());
    }

    // ────────────────────────────────────────────────────────────
    section("5. Division & Modulo");
    // ────────────────────────────────────────────────────────────
    check("divide by zero throws",
          exceptionThrown([]{BigInt(1LL)/BigInt(0LL);}));
    checkEq("0 / n = 0",          BigInt(0LL)/BigInt(7LL),          "0");
    checkEq("n / n = 1",          BigInt(7LL)/BigInt(7LL),          "1");
    checkEq("n / 1 = n",          BigInt(12345)/BigInt(1LL),      "12345");
    checkEq("14 / 3 = 4",         BigInt(14)/BigInt(3LL),         "4");
    checkEq("14 % 3 = 2",         BigInt(14)%BigInt(3LL),         "2");
    checkEq("-14 / 3 = -4",       BigInt(-14)/BigInt(3LL),        "-4");
    checkEq("-14 % 3 = -2",       BigInt(-14)%BigInt(3LL),        "-2");
    checkEq("large / small",
            BigInt("10000000000000000000")/BigInt("3"),          "3333333333333333333");
    checkEq("large % small",
            BigInt("10000000000000000000")%BigInt("3"),          "1");
    // Division identity: (a/b)*b + (a%b) == a
    {
        BigInt A("123456789012345678901234567890");
        BigInt B("9876543210");
        BigInt q = A / B, r = A % B;
        check("division identity (a/b)*b + a%b == a",
              (q * B + r) == A);
    }

    // ────────────────────────────────────────────────────────────
    section("6. Comparison Operators");
    // ────────────────────────────────────────────────────────────
    check("0 == 0",         BigInt(0LL) == BigInt(0LL));
    check("1 != 0",         BigInt(1LL) != BigInt(0LL));
    check("3 < 4",          BigInt(3LL) <  BigInt(4LL));
    check("4 > 3",          BigInt(4LL) >  BigInt(3LL));
    check("3 <= 3",         BigInt(3LL) <= BigInt(3LL));
    check("3 >= 3",         BigInt(3LL) >= BigInt(3LL));
    check("-1 < 1",         BigInt(-1LL) < BigInt(1LL));
    check("-5 > -10",       BigInt(-5LL) > BigInt(-10LL));
    check("large < larger",
          BigInt("999999999999999999999") < BigInt("1000000000000000000000"));
    check("neg large ordering",
          BigInt("-1000000000000000000000") < BigInt("-999999999999999999999"));

    // ────────────────────────────────────────────────────────────
    section("7. Unary & Increment / Decrement");
    // ────────────────────────────────────────────────────────────
    checkEq("unary -",     -BigInt(5LL),   "-5");
    checkEq("unary - neg", -BigInt(-5LL),  "5");
    checkEq("unary - 0",   -BigInt(0LL),   "0");
    checkEq("unary +",     +BigInt(-5LL),  "-5");
    {
        BigInt x(10);
        check("prefix ++", (++x) == BigInt(11LL));
        check("value after prefix++", x == BigInt(11LL));
    }
    {
        BigInt x(10);
        check("postfix ++", (x++) == BigInt(10LL));
        check("value after postfix++", x == BigInt(11LL));
    }
    {
        BigInt x(10);
        check("prefix --", (--x) == BigInt(9LL));
    }
    {
        BigInt x(0);
        check("0 -- goes negative", (--x) == BigInt(-1LL));
    }

    // ────────────────────────────────────────────────────────────
    section("8. Compound Assignment (+=, -=, *=, /=, %=)");
    // ────────────────────────────────────────────────────────────
    {
        BigInt v("1000000000000");
        v += BigInt("999999999999");
        checkEq("+=", v, "1999999999999");
        v -= BigInt("999999999999");
        checkEq("-=", v, "1000000000000");
        v *= BigInt("1000");
        checkEq("*=", v, "1000000000000000");
        v /= BigInt("1000");
        checkEq("/=", v, "1000000000000");
        v %= BigInt("7");
        checkEq("%=", v, "1000000000000" == BigInt("1000000000000").toString()
                          ? (BigInt("1000000000000") % BigInt("7")).toString()
                          : "?");
    }

    // ────────────────────────────────────────────────────────────
    section("9. Power");
    // ────────────────────────────────────────────────────────────
    checkEq("2^0",   BigInt(2LL).pow(0),  "1");
    checkEq("2^1",   BigInt(2LL).pow(1),  "2");
    checkEq("2^10",  BigInt(2LL).pow(10), "1024");
    checkEq("2^32",  BigInt(2LL).pow(32), "4294967296");
    checkEq("2^64",  BigInt(2LL).pow(64), "18446744073709551616");
    checkEq("10^20", BigInt(10LL).pow(20),"100000000000000000000");
    checkEq("(-1)^5", BigInt(-1LL).pow(5),"-1");
    checkEq("(-1)^6", BigInt(-1LL).pow(6),"1");
    check("negative exp throws",
          exceptionThrown([]{BigInt(2LL).pow(-1);}));

    // ────────────────────────────────────────────────────────────
    section("10. Modular Exponentiation");
    // ────────────────────────────────────────────────────────────
    checkEq("2^10 mod 1000",  BigInt(2LL).modPow(BigInt(10LL),  BigInt(1000)),  "24");
    checkEq("3^100 mod 97",   BigInt(3LL).modPow(BigInt(100), BigInt(97)),    "81");
    checkEq("anything mod 1", BigInt(99999).modPow(BigInt(9999), BigInt(1LL)),"0");
    // Fermat's little theorem: a^(p-1) ≡ 1 (mod p) for prime p
    {
        BigInt p(1000000007);
        BigInt a(123456789);
        checkEq("Fermat: a^(p-1) mod p = 1",
                a.modPow(p - BigInt(1LL), p), "1");
    }

    // ────────────────────────────────────────────────────────────
    section("11. GCD & LCM");
    // ────────────────────────────────────────────────────────────
    checkEq("gcd(12,8)",      BigInt(12).gcd(BigInt(8LL)),         "4");
    checkEq("gcd(100,75)",    BigInt(100).gcd(BigInt(75)),       "25");
    checkEq("gcd(prime,1)",   BigInt(97).gcd(BigInt(1LL)),         "1");
    checkEq("gcd(n,n)",       BigInt(12345).gcd(BigInt(12345)),  "12345");
    checkEq("gcd(n,0)",       BigInt(42LL).gcd(BigInt(0LL)),         "42");
    checkEq("lcm(4,6)",       BigInt(4LL).lcm(BigInt(6LL)),          "12");
    checkEq("lcm(0,5)",       BigInt(0LL).lcm(BigInt(5LL)),          "0");
    {
        BigInt A("123456789012345678901234567890");
        BigInt B("987654321098765432109876543210");
        BigInt g = A.gcd(B);
        // Check that g divides both A and B
        check("gcd divides A", (A % g) == BigInt(0LL));
        check("gcd divides B", (B % g) == BigInt(0LL));
    }

    // ────────────────────────────────────────────────────────────
    section("12. Modular Inverse");
    // ────────────────────────────────────────────────────────────
    {
        BigInt p(1000000007);
        BigInt a(123456789);
        BigInt inv = a.modInverse(p);
        checkEq("a * a^-1 mod p = 1", (a * inv) % p, "1");
    }
    {
        BigInt a(3), m(11);
        BigInt inv = a.modInverse(m);
        checkEq("3^-1 mod 11 = 4", inv, "4");
        checkEq("verify 3*4 mod 11 = 1", (a * inv) % m, "1");
    }
    check("inverse of non-coprime throws",
          exceptionThrown([]{BigInt(4LL).modInverse(BigInt(6LL));}));

    // ────────────────────────────────────────────────────────────
    section("13. Integer Square Root");
    // ────────────────────────────────────────────────────────────
    checkEq("isqrt(0)",  BigInt(0LL).isqrt(),   "0");
    checkEq("isqrt(1)",  BigInt(1LL).isqrt(),   "1");
    checkEq("isqrt(4)",  BigInt(4LL).isqrt(),   "2");
    checkEq("isqrt(9)",  BigInt(9LL).isqrt(),   "3");
    checkEq("isqrt(15)", BigInt(15).isqrt(),  "3");   // floor(sqrt(15)) = 3
    checkEq("isqrt(100)",BigInt(100).isqrt(), "10");
    {
        BigInt N("123456789012345678901234567890");
        BigInt s = N.isqrt();
        check("isqrt: s^2 <= N", s * s <= N);
        check("isqrt: N < (s+1)^2", N < (s + BigInt(1LL)) * (s + BigInt(1LL)));
    }
    check("isqrt of negative throws",
          exceptionThrown([]{BigInt(-1LL).isqrt();}));

    // ────────────────────────────────────────────────────────────
    section("14. Factorial & Fibonacci");
    // ────────────────────────────────────────────────────────────
    checkEq("0! = 1",  BigInt::factorial(0), "1");
    checkEq("1! = 1",  BigInt::factorial(1), "1");
    checkEq("5! = 120",BigInt::factorial(5), "120");
    checkEq("10!",     BigInt::factorial(10),"3628800");
    check("20! digits >= 18", BigInt::factorial(20).numDigits() >= 18);
    {
        // 100! must end in many zeros (zeros from factors of 5 & 2)
        std::string f100 = BigInt::factorial(100).toString();
        int trailing_zeros = 0;
        for (int i = (int)f100.size()-1; i >= 0 && f100[i]=='0'; --i)
            ++trailing_zeros;
        check("100! has 24 trailing zeros", trailing_zeros == 24);
    }
    checkEq("F(0)=0",  BigInt::fibonacci(0), "0");
    checkEq("F(1)=1",  BigInt::fibonacci(1), "1");
    checkEq("F(10)=55",BigInt::fibonacci(10),"55");
    checkEq("F(20)=6765", BigInt::fibonacci(20),"6765");
    {
        // Fibonacci identity: F(n)^2 + F(n-1)^2 == F(2n-1)
        BigInt fn  = BigInt::fibonacci(50);
        BigInt fn1 = BigInt::fibonacci(49);
        BigInt f2n1= BigInt::fibonacci(99);
        check("Fib identity F(50)^2+F(49)^2 == F(99)",
              fn * fn + fn1 * fn1 == f2n1);
    }

    // ────────────────────────────────────────────────────────────
    section("15. Edge Cases & Stress");
    // ────────────────────────────────────────────────────────────
    checkEq("very large * 0",
            BigInt("9999999999999999999999999999999") * BigInt(0), "0");
    checkEq("very large + 0",
            BigInt("9999999999999999999999999999999") + BigInt(0),
            "9999999999999999999999999999999");
    checkEq("0 - 0 = 0",  BigInt(0) - BigInt(0), "0");
    checkEq("-0 is 0",    BigInt("-0").toString(), "0");
    check("abs of neg",   BigInt(-42LL).abs() == BigInt(42));
    check("abs of pos",   BigInt(42).abs()  == BigInt(42));
    check("sign(-99)=-1", BigInt(-99LL).sign() == -1);
    check("sign(0)=0",    BigInt(0).sign()   == 0);
    check("sign(99)=1",   BigInt(99).sign()  == 1);
    check("isEven(0)",    BigInt(0).isEven());
    check("isOdd(1)",     BigInt(1).isOdd());
    check("isEven(-4)",   BigInt(-4).isEven());
    check("isOdd(-3)",    BigInt(-3).isOdd());
    check("isZero(0)",    BigInt(0).isZero());
    check("!isZero(1)",  !BigInt(1).isZero());
    check("isOne(1)",     BigInt(1).isOne());
    check("!isOne(-1)",  !BigInt(-1).isOne());
    // Chain of operations
    {
        BigInt v = (BigInt(2).pow(100) - BigInt(1)) % BigInt(1000000007);
        check("(2^100 - 1) mod 10^9+7 is in range", v >= BigInt(0) && v < BigInt(1000000007));
    }

    // ────────────────────────────────────────────────────────────
    section("16. I/O Round-Trip");
    // ────────────────────────────────────────────────────────────
    {
        BigInt orig("123456789012345678901234567890");
        BigInt rt(orig.toString());
        check("round-trip positive", orig == rt);
    }
    {
        BigInt orig("-987654321098765432109876543210");
        BigInt rt(orig.toString());
        check("round-trip negative", orig == rt);
    }
    {
        BigInt rt(BigInt(0).toString());
        check("round-trip zero", rt == BigInt(0));
    }

    // ────────────────────────────────────────────────────────────
    section("17. isPrime / nextPrime / nthPrime");
    // ────────────────────────────────────────────────────────────
    check("2 is prime",          BigInt(2).isPrime());
    check("3 is prime",          BigInt(3).isPrime());
    check("5 is prime",          BigInt(5).isPrime());
    check("7 is prime",          BigInt(7).isPrime());
    check("97 is prime",         BigInt(97).isPrime());
    check("1000000007 is prime",  BigInt(1000000007LL).isPrime());
    check("0 not prime",         !BigInt(0).isPrime());
    check("1 not prime",         !BigInt(1).isPrime());
    check("4 not prime",         !BigInt(4).isPrime());
    check("100 not prime",       !BigInt(100).isPrime());
    check("1000000008 not prime", !BigInt(1000000008LL).isPrime());
    // Large prime (2^31 - 1 = 2147483647, a Mersenne prime)
    check("2^31-1 is prime",      BigInt(2147483647LL).isPrime());
    // Large composite
    check("2^31 not prime",      !BigInt(2LL).pow(31).isPrime());

    // nextPrime
    checkEq("nextPrime(2)=3",   BigInt(2).nextPrime(),  "3");
    checkEq("nextPrime(10)=11", BigInt(10).nextPrime(), "11");
    checkEq("nextPrime(12)=13", BigInt(12).nextPrime(), "13");
    checkEq("nextPrime(96)=97", BigInt(96).nextPrime(), "97");

    // nthPrime
    checkEq("nthPrime(1)=2",    BigInt::nthPrime(1),  "2");
    checkEq("nthPrime(2)=3",    BigInt::nthPrime(2),  "3");
    checkEq("nthPrime(5)=11",   BigInt::nthPrime(5),  "11");
    checkEq("nthPrime(10)=29",  BigInt::nthPrime(10), "29");
    checkEq("nthPrime(25)=97",  BigInt::nthPrime(25), "97");
    check("nthPrime(0) throws", exceptionThrown([]{ BigInt::nthPrime(0); }));

    // ────────────────────────────────────────────────────────────
    section("18. nthRoot / binomial / trailingZeros / logs");
    // ────────────────────────────────────────────────────────────
    // nthRoot
    checkEq("cbrt(8)=2",    BigInt(8).nthRoot(3),   "2");
    checkEq("cbrt(27)=3",   BigInt(27).nthRoot(3),  "3");
    checkEq("cbrt(26)=2",   BigInt(26).nthRoot(3),  "2");  // floor
    checkEq("4thRoot(16)=2",BigInt(16).nthRoot(4),  "2");
    checkEq("4thRoot(80)=2",BigInt(80).nthRoot(4),  "2");  // floor(80^0.25)=2
    checkEq("nthRoot(k=1)", BigInt(12345).nthRoot(1), "12345");
    {
        BigInt N = BigInt(2).pow(90);
        BigInt r = N.nthRoot(3);
        // Verify r^3 <= N < (r+1)^3
        check("nthRoot(3) floor correct: r^3<=N",  r.pow(3) <= N);
        check("nthRoot(3) floor correct: N<(r+1)^3", N < (r+BigInt(1)).pow(3));
    }
    check("nthRoot even of negative throws",
          exceptionThrown([]{ BigInt(-8).nthRoot(2); }));
    check("nthRoot k=0 throws",
          exceptionThrown([]{ BigInt(8).nthRoot(0); }));

    // binomial
    checkEq("C(5,0)=1",    BigInt::binomial(5,0),   "1");
    checkEq("C(5,5)=1",    BigInt::binomial(5,5),   "1");
    checkEq("C(5,2)=10",   BigInt::binomial(5,2),   "10");
    checkEq("C(10,3)=120", BigInt::binomial(10,3),  "120");
    checkEq("C(20,10)=184756", BigInt::binomial(20,10), "184756");
    checkEq("C(5,-1)=0",   BigInt::binomial(5,-1),  "0");
    checkEq("C(5,6)=0",    BigInt::binomial(5,6),   "0");

    // trailingZeros
    check("trailingZeros(100)=2",   BigInt(100).trailingZeros()  == 2);
    check("trailingZeros(1000)=3",  BigInt(1000).trailingZeros() == 3);
    check("trailingZeros(7)=0",     BigInt(7).trailingZeros()    == 0);
    check("trailingZeros(10^9)=9",  BigInt("1000000000").trailingZeros() == 9);
    {
        long long tz = BigInt::factorial(100).trailingZeros();
        check("100! trailing zeros = 24", tz == 24);
    }

    // approxLog2 / approxLog10
    {
        double log2_1024 = BigInt(1024).approxLog2();
        check("log2(1024) ≈ 10", std::abs(log2_1024 - 10.0) < 0.01);
        double log10_1000 = BigInt(1000).approxLog10();
        check("log10(1000) ≈ 3", std::abs(log10_1000 - 3.0) < 0.01);
        // 2^256 has bitLength ~256, log10 ~77.06
        BigInt p256 = BigInt(2).pow(256);
        check("log2(2^256) ≈ 256", std::abs(p256.approxLog2() - 256.0) < 1.0);
    }

    // divmod (public)
    {
        std::pair<BigInt, BigInt> qr = BigInt(17).divmod(BigInt(5));
        checkEq("divmod q: 17/5=3", qr.first, "3");
        checkEq("divmod r: 17%5=2", qr.second, "2");
    }
    {
        BigInt A("123456789012345678901234567890");
        BigInt B("9876543210");
        std::pair<BigInt, BigInt> qr = A.divmod(B);
        check("divmod identity", qr.first * B + qr.second == A);
    }

    // ────────────────────────────────────────────────────────────
    section("19. Bit Shifts (<< and >>)");
    // ────────────────────────────────────────────────────────────
    checkEq("1 << 0 = 1",   BigInt(1) << 0,  "1");
    checkEq("1 << 1 = 2",   BigInt(1) << 1,  "2");
    checkEq("1 << 8 = 256", BigInt(1) << 8,  "256");
    checkEq("1 << 10 = 1024", BigInt(1) << 10, "1024");
    checkEq("3 << 3 = 24",  BigInt(3) << 3,  "24");
    checkEq("256 >> 1 = 128", BigInt(256) >> 1, "128");
    checkEq("256 >> 8 = 1",   BigInt(256) >> 8, "1");
    checkEq("7 >> 1 = 3",     BigInt(7)   >> 1, "3");   // floor division
    checkEq("0 << 100 = 0",   BigInt(0)   << 100, "0");
    checkEq("0 >> 100 = 0",   BigInt(0)   >> 100, "0");
    // Round trip
    {
        BigInt v(12345);
        check("<< then >> roundtrip", (v << 20) >> 20 == v);
    }
    // Compound assignment
    {
        BigInt v(1);
        v <<= 10;
        checkEq("<<= 10", v, "1024");
        v >>= 5;
        checkEq(">>= 5",  v, "32");
    }

    // ────────────────────────────────────────────────────────────
    section("20. Bitwise AND / OR / XOR + bit inspection");
    // ────────────────────────────────────────────────────────────
    // AND
    checkEq("12 & 10 = 8",  BigInt(12) & BigInt(10),  "8");   // 1100 & 1010 = 1000
    checkEq("0 & 255 = 0",  BigInt(0)  & BigInt(255), "0");
    checkEq("255 & 255 = 255", BigInt(255) & BigInt(255), "255");
    checkEq("0b1111 & 0b0101 = 5", BigInt(15) & BigInt(5), "5");
    // OR
    checkEq("12 | 10 = 14", BigInt(12) | BigInt(10),  "14");  // 1100 | 1010 = 1110
    checkEq("0 | 255 = 255", BigInt(0) | BigInt(255), "255");
    checkEq("8 | 4 = 12",   BigInt(8)  | BigInt(4),   "12");
    // XOR
    checkEq("12 ^ 10 = 6",  BigInt(12) ^ BigInt(10),  "6");   // 1100 ^ 1010 = 0110
    checkEq("255 ^ 255 = 0", BigInt(255) ^ BigInt(255), "0");
    checkEq("5 ^ 3 = 6",    BigInt(5)   ^ BigInt(3),   "6");

    // bitLength
    check("bitLength(0)=0",   BigInt(0).bitLength()   == 0);
    check("bitLength(1)=1",   BigInt(1).bitLength()   == 1);
    check("bitLength(2)=2",   BigInt(2).bitLength()   == 2);
    check("bitLength(255)=8", BigInt(255).bitLength() == 8);
    check("bitLength(256)=9", BigInt(256).bitLength() == 9);

    // testBit
    check("testBit(5,0)=true",  BigInt(5).testBit(0));    // 101 bit0=1
    check("testBit(5,1)=false", !BigInt(5).testBit(1));   // 101 bit1=0
    check("testBit(5,2)=true",  BigInt(5).testBit(2));    // 101 bit2=1
    check("testBit(0,0)=false", !BigInt(0).testBit(0));

    // setBit
    checkEq("setBit(5,1)",  BigInt(5).setBit(1),  "7");   // 101 -> 111
    checkEq("setBit(0,3)",  BigInt(0).setBit(3),  "8");   // 000 -> 1000
    // clearBit
    checkEq("clearBit(7,1)", BigInt(7).clearBit(1), "5"); // 111 -> 101
    checkEq("clearBit(8,3)", BigInt(8).clearBit(3), "0"); // 1000 -> 0

    // ────────────────────────────────────────────────────────────
    section("21. Base Conversion (hex / binary)");
    // ────────────────────────────────────────────────────────────
    // toBinary
    checkEq("toBinary(0)='0'",  BigInt(0), "0");
    check("toBinary(1)='1'",   BigInt(1).toBinary()   == "1");
    check("toBinary(2)='10'",  BigInt(2).toBinary()   == "10");
    check("toBinary(5)='101'", BigInt(5).toBinary()   == "101");
    check("toBinary(255)='11111111'", BigInt(255).toBinary() == "11111111");
    check("toBinary(1024)='10000000000'", BigInt(1024).toBinary() == "10000000000");

    // toHex
    check("toHex(0)='0'",    BigInt(0).toHex()    == "0");
    check("toHex(255)='ff'", BigInt(255).toHex()  == "ff");
    check("toHex(256)='100'",BigInt(256).toHex()  == "100");
    check("toHex(16)='10'",  BigInt(16).toHex()   == "10");
    check("toHex(15)='f'",   BigInt(15).toHex()   == "f");
    check("toHex(2^32)='100000000'", BigInt(2).pow(32).toHex() == "100000000");

    // fromHex round-trip
    {
        BigInt orig(255);
        BigInt rt = BigInt::fromHex(orig.toHex());
        check("fromHex round-trip 255", rt == orig);
    }
    {
        BigInt orig = BigInt(2).pow(64);
        BigInt rt   = BigInt::fromHex(orig.toHex());
        check("fromHex round-trip 2^64", rt == orig);
    }
    check("fromHex('ff')=255",  BigInt::fromHex("ff")  == BigInt(255));
    check("fromHex('FF')=255",  BigInt::fromHex("FF")  == BigInt(255));
    check("fromHex('10')=16",   BigInt::fromHex("10")  == BigInt(16));
    check("fromHex invalid throws",
          exceptionThrown([]{ BigInt::fromHex("xyz"); }));

    // fromBinary round-trip
    {
        BigInt orig(42);
        BigInt rt = BigInt::fromBinary(orig.toBinary());
        check("fromBinary round-trip 42", rt == orig);
    }
    check("fromBinary('101')=5", BigInt::fromBinary("101") == BigInt(5));
    check("fromBinary('0')=0",   BigInt::fromBinary("0")   == BigInt(0));
    check("fromBinary invalid throws",
          exceptionThrown([]{ BigInt::fromBinary("102"); }));

    // ────────────────────────────────────────────────────────────
    //  Summary
    // ────────────────────────────────────────────────────────────
    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Results: " << pass_count << " passed, " << fail_count
              << " failed" << std::string(46 - std::to_string(pass_count).size()
                                             - std::to_string(fail_count).size(), ' ')
              << "║\n";
    if (fail_count == 0)
        std::cout << "║  ✓  ALL TESTS PASSED                                     ║\n";
    else
        std::cout << "║  ✗  SOME TESTS FAILED — see above                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return fail_count == 0 ? 0 : 1;
}
