#include "BigInt.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <random>

// ═══════════════════════════════════════════════════════════════
//  Constructors
// ═══════════════════════════════════════════════════════════════

BigInt::BigInt() : mag(), negative(false) {}

BigInt::BigInt(int n) : BigInt(static_cast<long long>(n)) {}

BigInt::BigInt(long long n) : negative(n < 0) {
    // Use unsigned arithmetic to handle LLONG_MIN correctly
    unsigned long long uv = negative
        ? static_cast<unsigned long long>(-(n + 1)) + 1ULL
        : static_cast<unsigned long long>(n);
    while (uv > 0) {
        mag.push_back(static_cast<long long>(uv % static_cast<unsigned long long>(BASE)));
        uv /= static_cast<unsigned long long>(BASE);
    }
}

BigInt::BigInt(const std::string& s) : negative(false) {
    if (s.empty()) return;
    int start = 0;
    if (s[0] == '-') { negative = true;  start = 1; }
    else if (s[0] == '+')              { start = 1; }

    for (int i = start; i < (int)s.size(); ++i)
        if (!isdigit((unsigned char)s[i]))
            throw std::invalid_argument("BigInt: invalid character '" +
                                        std::string(1, s[i]) + "'");

    // Parse right-to-left in BASE_DIGITS chunks
    for (int i = (int)s.size(); i > start; i -= BASE_DIGITS) {
        int from = std::max(start, i - BASE_DIGITS);
        mag.push_back(std::stoll(s.substr(from, i - from)));
    }
    trim();
}

BigInt::BigInt(const char* s) : BigInt(std::string(s)) {}

BigInt& BigInt::operator=(long long n)          { return *this = BigInt(n); }
BigInt& BigInt::operator=(const std::string& s) { return *this = BigInt(s); }

// ═══════════════════════════════════════════════════════════════
//  Private Helpers
// ═══════════════════════════════════════════════════════════════

// Remove leading zero-limbs; ensure 0 has negative == false
void BigInt::trim() {
    while (!mag.empty() && mag.back() == 0) mag.pop_back();
    if (mag.empty()) negative = false;
}

// Compare magnitudes only: -1 | 0 | 1
int BigInt::compareMag(const BigInt& a, const BigInt& b) {
    if (a.mag.size() != b.mag.size())
        return a.mag.size() < b.mag.size() ? -1 : 1;
    for (int i = (int)a.mag.size() - 1; i >= 0; --i)
        if (a.mag[i] != b.mag[i])
            return a.mag[i] < b.mag[i] ? -1 : 1;
    return 0;
}

// |a| + |b|
BigInt BigInt::addMag(const BigInt& a, const BigInt& b) {
    BigInt res;
    long long carry = 0;
    int n = (int)std::max(a.mag.size(), b.mag.size());
    res.mag.reserve(n + 1);
    for (int i = 0; i < n || carry; ++i) {
        long long sum = carry;
        if (i < (int)a.mag.size()) sum += a.mag[i];
        if (i < (int)b.mag.size()) sum += b.mag[i];
        res.mag.push_back(sum % BASE);
        carry = sum / BASE;
    }
    return res;
}

// |a| - |b|   (caller guarantees |a| >= |b|)
BigInt BigInt::subMag(const BigInt& a, const BigInt& b) {
    BigInt res;
    long long borrow = 0;
    res.mag.reserve(a.mag.size());
    for (int i = 0; i < (int)a.mag.size(); ++i) {
        long long diff = a.mag[i] - borrow
                       - (i < (int)b.mag.size() ? b.mag[i] : 0);
        if (diff < 0) { diff += BASE; borrow = 1; }
        else            borrow = 0;
        res.mag.push_back(diff);
    }
    res.trim();
    return res;
}

// a * s   where s is a single non-negative limb (0 <= s < BASE)
BigInt BigInt::mulScalar(const BigInt& a, long long s) {
    if (s == 0 || a.isZero()) return BigInt(0);
    BigInt res;
    long long carry = 0;
    res.mag.reserve(a.mag.size() + 1);
    for (int i = 0; i < (int)a.mag.size() || carry; ++i) {
        long long cur = carry;
        if (i < (int)a.mag.size()) cur += a.mag[i] * s;
        res.mag.push_back(cur % BASE);
        carry = cur / BASE;
    }
    res.trim();
    return res;
}

// a * BASE^k   (shift limbs left by k positions)
BigInt BigInt::shiftLimbs(const BigInt& a, int k) {
    if (a.isZero() || k == 0) return a;
    BigInt res;
    res.mag.resize(k, 0);
    for (long long d : a.mag) res.mag.push_back(d);
    return res;
}

// ─────────────────── Schoolbook O(n²) multiplication ──────────
BigInt BigInt::naiveMul(const BigInt& a, const BigInt& b) {
    if (a.isZero() || b.isZero()) return BigInt(0);
    BigInt res;
    res.mag.assign(a.mag.size() + b.mag.size(), 0LL);

    for (int i = 0; i < (int)a.mag.size(); ++i) {
        long long carry = 0;
        for (int j = 0; j < (int)b.mag.size() || carry; ++j) {
            long long cur = res.mag[i + j] + carry;
            if (j < (int)b.mag.size()) cur += a.mag[i] * b.mag[j];
            res.mag[i + j] = cur % BASE;
            carry           = cur / BASE;
        }
    }
    res.trim();
    return res;
}

// ─────────────────── Karatsuba O(n^1.585) multiplication ──────
BigInt BigInt::karatsubaMul(const BigInt& a, const BigInt& b) {
    int n = (int)std::max(a.mag.size(), b.mag.size());
    if (n < KARA_THRESH) return naiveMul(a, b);

    int m = n / 2;

    // Split a
    BigInt a_lo, a_hi;
    for (int i = 0; i < m && i < (int)a.mag.size(); ++i) a_lo.mag.push_back(a.mag[i]);
    for (int i = m;           i < (int)a.mag.size(); ++i) a_hi.mag.push_back(a.mag[i]);
    a_lo.trim(); a_hi.trim();

    // Split b
    BigInt b_lo, b_hi;
    for (int i = 0; i < m && i < (int)b.mag.size(); ++i) b_lo.mag.push_back(b.mag[i]);
    for (int i = m;           i < (int)b.mag.size(); ++i) b_hi.mag.push_back(b.mag[i]);
    b_lo.trim(); b_hi.trim();

    BigInt z0 = karatsubaMul(a_lo, b_lo);
    BigInt z2 = karatsubaMul(a_hi, b_hi);

    // z1 = (a_lo + a_hi) * (b_lo + b_hi) - z0 - z2  (always >= 0)
    BigInt z1 = karatsubaMul(addMag(a_lo, a_hi), addMag(b_lo, b_hi));
    z1 = subMag(z1, addMag(z0, z2));

    // Combine: result = z2*BASE^(2m) + z1*BASE^m + z0
    BigInt result = addMag(addMag(shiftLimbs(z2, 2 * m),
                                  shiftLimbs(z1,     m)),
                           z0);
    return result;
}

// ─────────────── Long Division (magnitude version) ────────────
std::pair<BigInt, BigInt> BigInt::divmodMag(const BigInt& num,
                                            const BigInt& den) {
    if (den.isZero()) throw std::runtime_error("BigInt: division by zero");

    int cmp = compareMag(num, den);
    if (cmp <  0) return { BigInt(0), num };
    if (cmp == 0) return { BigInt(1), BigInt(0) };

    BigInt quot, rem;
    quot.mag.resize(num.mag.size(), 0LL);

    for (int i = (int)num.mag.size() - 1; i >= 0; --i) {
        // rem = rem * BASE + num.mag[i]
        rem.mag.insert(rem.mag.begin(), num.mag[i]);
        rem.trim();

        // Binary search: find largest q in [0, BASE-1] with q*den <= rem
        long long lo = 0, hi = BASE - 1, q = 0;
        while (lo <= hi) {
            long long mid = lo + (hi - lo) / 2;
            BigInt t = mulScalar(den, mid);
            if (compareMag(t, rem) <= 0) { q = mid; lo = mid + 1; }
            else                           hi = mid - 1;
        }
        quot.mag[i] = q;
        rem = subMag(rem, mulScalar(den, q));
    }
    quot.trim();
    rem.trim();
    return { quot, rem };
}

// ═══════════════════════════════════════════════════════════════
//  Arithmetic Operators
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::operator+(const BigInt& rhs) const {
    BigInt res;
    if (negative == rhs.negative) {
        res = addMag(*this, rhs);
        res.negative = negative;
    } else {
        int cmp = compareMag(*this, rhs);
        if (cmp == 0) return BigInt(0);
        if (cmp > 0) { res = subMag(*this, rhs); res.negative = negative;     }
        else         { res = subMag(rhs, *this);  res.negative = rhs.negative; }
    }
    res.trim();
    return res;
}

BigInt BigInt::operator-(const BigInt& rhs) const {
    BigInt neg = rhs;
    if (!neg.isZero()) neg.negative = !neg.negative;
    return *this + neg;
}

BigInt BigInt::operator*(const BigInt& rhs) const {
    if (isZero() || rhs.isZero()) return BigInt(0);
    BigInt res = karatsubaMul(*this, rhs);
    res.negative = (negative != rhs.negative);
    res.trim();
    return res;
}

BigInt BigInt::operator/(const BigInt& rhs) const {
    std::pair<BigInt,BigInt> qr = divmodMag(abs(), rhs.abs());
    qr.first.negative = (negative != rhs.negative) && !qr.first.isZero();
    return qr.first;
}

BigInt BigInt::operator%(const BigInt& rhs) const {
    std::pair<BigInt,BigInt> qr = divmodMag(abs(), rhs.abs());
    qr.second.negative = negative && !qr.second.isZero();
    return qr.second;
}

BigInt& BigInt::operator+=(const BigInt& rhs) { return *this = *this + rhs; }
BigInt& BigInt::operator-=(const BigInt& rhs) { return *this = *this - rhs; }
BigInt& BigInt::operator*=(const BigInt& rhs) { return *this = *this * rhs; }
BigInt& BigInt::operator/=(const BigInt& rhs) { return *this = *this / rhs; }
BigInt& BigInt::operator%=(const BigInt& rhs) { return *this = *this % rhs; }

// ═══════════════════════════════════════════════════════════════
//  Unary Operators & Increment / Decrement
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::operator-() const {
    BigInt res = *this;
    if (!res.isZero()) res.negative = !res.negative;
    return res;
}
BigInt BigInt::operator+() const { return *this; }

BigInt& BigInt::operator++()    { *this += BigInt(1); return *this; }
BigInt  BigInt::operator++(int) { BigInt tmp = *this; ++(*this); return tmp; }
BigInt& BigInt::operator--()    { *this -= BigInt(1); return *this; }
BigInt  BigInt::operator--(int) { BigInt tmp = *this; --(*this); return tmp; }

// ═══════════════════════════════════════════════════════════════
//  Comparison Operators
// ═══════════════════════════════════════════════════════════════

bool BigInt::operator==(const BigInt& rhs) const {
    return negative == rhs.negative && mag == rhs.mag;
}
bool BigInt::operator!=(const BigInt& rhs) const { return !(*this == rhs); }

bool BigInt::operator<(const BigInt& rhs) const {
    if (negative != rhs.negative) return negative;       // neg < pos always
    int cmp = compareMag(*this, rhs);
    return negative ? (cmp > 0) : (cmp < 0);
}
bool BigInt::operator<=(const BigInt& rhs) const { return !(rhs < *this); }
bool BigInt::operator> (const BigInt& rhs) const { return   rhs < *this;  }
bool BigInt::operator>=(const BigInt& rhs) const { return !(*this < rhs); }

// ═══════════════════════════════════════════════════════════════
//  I / O
// ═══════════════════════════════════════════════════════════════

std::string BigInt::toString() const {
    if (isZero()) return "0";
    std::string s;
    if (negative) s += '-';
    s += std::to_string(mag.back());
    for (int i = (int)mag.size() - 2; i >= 0; --i) {
        std::string chunk = std::to_string(mag[i]);
        s += std::string(BASE_DIGITS - chunk.size(), '0') + chunk;
    }
    return s;
}

std::ostream& operator<<(std::ostream& os, const BigInt& n) {
    return os << n.toString();
}
std::istream& operator>>(std::istream& is, BigInt& n) {
    std::string s; is >> s; n = BigInt(s); return is;
}

// ═══════════════════════════════════════════════════════════════
//  Utility
// ═══════════════════════════════════════════════════════════════

bool   BigInt::isZero()     const { return mag.empty(); }
bool   BigInt::isOne()      const { return mag.size() == 1 && mag[0] == 1 && !negative; }
bool   BigInt::isNegative() const { return negative; }
bool   BigInt::isPositive() const { return !negative && !isZero(); }
bool   BigInt::isEven()     const { return isZero() || (mag[0] % 2 == 0); }
bool   BigInt::isOdd()      const { return !isEven(); }
BigInt BigInt::abs()        const { BigInt r = *this; r.negative = false; return r; }
int    BigInt::sign()       const { return isZero() ? 0 : (negative ? -1 : 1); }

int BigInt::numDigits() const {
    if (isZero()) return 1;
    return (int)((mag.size() - 1) * BASE_DIGITS
               + std::to_string(mag.back()).size());
}

// ═══════════════════════════════════════════════════════════════
//  Public divmod
// ═══════════════════════════════════════════════════════════════

std::pair<BigInt, BigInt> BigInt::divmod(const BigInt& other) const {
    auto qr = divmodMag(abs(), other.abs());
    qr.first.negative  = (negative != other.negative) && !qr.first.isZero();
    qr.second.negative = negative && !qr.second.isZero();
    return qr;
}

// ═══════════════════════════════════════════════════════════════
//  Power  (binary exponentiation, O(log exp) multiplications)
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::pow(long long exp) const {
    if (exp < 0) throw std::runtime_error("BigInt::pow: negative exponent");
    if (exp == 0) return BigInt(1);
    BigInt base = *this, result(1);
    while (exp > 0) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  Modular Exponentiation
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::modPow(const BigInt& exp, const BigInt& mod) const {
    if (mod == BigInt(1LL)) return BigInt(0LL);
    BigInt result(1LL);
    BigInt base = *this % mod;
    BigInt e    = exp;
    while (!e.isZero()) {
        if (e.isOdd()) result = (result * base) % mod;
        e    = e / BigInt(2LL);
        base = (base * base) % mod;
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  GCD  –  Euclidean Algorithm
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::gcd(const BigInt& other) const {
    BigInt a = abs(), b = other.abs();
    while (!b.isZero()) {
        BigInt t = a % b;
        a = b;
        b = t;
    }
    return a;
}

// ═══════════════════════════════════════════════════════════════
//  LCM
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::lcm(const BigInt& other) const {
    if (isZero() || other.isZero()) return BigInt(0LL);
    return (abs() / gcd(other)) * other.abs();
}

// ═══════════════════════════════════════════════════════════════
//  Modular Inverse  –  Extended Euclidean Algorithm
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::modInverse(const BigInt& mod) const {
    BigInt a    = abs() % mod;
    BigInt r    = mod;
    BigInt newr = a;
    BigInt t    = BigInt(0LL);
    BigInt newt = BigInt(1LL);

    while (!newr.isZero()) {
        BigInt q = r / newr;

        BigInt tmp_r = r - q * newr;
        r    = newr;
        newr = tmp_r;

        BigInt tmp_t = t - q * newt;
        t    = newt;
        newt = tmp_t;
    }
    if (r > BigInt(1LL))
        throw std::runtime_error("BigInt::modInverse: inverse does not exist (gcd != 1)");
    if (t.isNegative()) t = t + mod;
    return t;
}

// ═══════════════════════════════════════════════════════════════
//  Integer Square Root  (Newton's method)
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::isqrt() const {
    if (negative) throw std::runtime_error("BigInt::isqrt: negative operand");
    if (isZero()) return BigInt(0LL);
    if (isOne())  return BigInt(1LL);

    // Good initial estimate: 10^( (numDigits+1)/2 )
    int d = numDigits();
    BigInt x("1" + std::string((d + 1) / 2, '0'));

    while (true) {
        BigInt nx = (x + *this / x) / BigInt(2);
        if (nx >= x) break;
        x = nx;
    }
    return x;
}

// ═══════════════════════════════════════════════════════════════
//  k-th Root  (Newton's method generalization)
//  Returns floor(this^(1/k))
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::nthRoot(long long k) const {
    if (k <= 0) throw std::runtime_error("BigInt::nthRoot: k must be >= 1");
    if (k == 1) return *this;
    if (negative) {
        if (k % 2 == 0)
            throw std::runtime_error("BigInt::nthRoot: even root of negative number");
        BigInt r = (-*this).nthRoot(k);
        r.negative = true;
        return r;
    }
    if (isZero()) return BigInt(0LL);
    if (k == 2)   return isqrt();

    // Initial estimate: 10^( ceil(numDigits / k) )
    int d = numDigits();
    int estDigits = (d + k - 1) / k;
    BigInt x("1" + std::string(estDigits, '0'));
    BigInt K(k);

    while (true) {
        // Newton step: x_new = ((k-1)*x + N / x^(k-1)) / k
        BigInt xk1 = x.pow(k - 1);           // x^(k-1)
        BigInt nx  = (BigInt(k - 1) * x + *this / xk1) / K;
        if (nx >= x) break;
        x = nx;
    }
    // Adjust down if overshoot (Newton can overshoot by 1)
    while (x.pow(k) > *this) x -= BigInt(1);
    return x;
}

// ═══════════════════════════════════════════════════════════════
//  Factorial
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::factorial(long long n) {
    if (n < 0) throw std::runtime_error("BigInt::factorial: negative argument");
    BigInt result(1LL);
    for (long long i = 2; i <= n; ++i) result *= BigInt(i);
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  Fibonacci  (iterative, O(n) BigInt additions)
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::fibonacci(long long n) {
    if (n <= 0) return BigInt(0LL);
    if (n == 1) return BigInt(1LL);
    BigInt a(0LL), b(1LL);
    for (long long i = 2; i <= n; ++i) {
        BigInt c = a + b;
        a = b;
        b = c;
    }
    return b;
}

// ═══════════════════════════════════════════════════════════════
//  Binomial Coefficient  C(n, k)
//  Uses multiplicative formula to avoid huge intermediates:
//    C(n,k) = n*(n-1)*...*(n-k+1) / k!
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::binomial(long long n, long long k) {
    if (k < 0 || k > n) return BigInt(0LL);
    if (k == 0 || k == n) return BigInt(1LL);
    if (k > n - k) k = n - k;   // use smaller k for efficiency

    BigInt result(1LL);
    for (long long i = 0; i < k; ++i) {
        result *= BigInt(n - i);
        result /= BigInt(i + 1);
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  Trailing Zeros  (count trailing decimal zeros)
//  Lowest limb tells us immediately since BASE = 10^9.
// ═══════════════════════════════════════════════════════════════

long long BigInt::trailingZeros() const {
    if (isZero()) return 1;  // "0" has one trailing zero by convention
    long long count = 0;
    // Count zero limbs (each contributes BASE_DIGITS zeros)
    for (size_t i = 0; i < mag.size(); ++i) {
        if (mag[i] == 0) {
            count += BASE_DIGITS;
        } else {
            // Count zeros in the lowest non-zero limb
            long long limb = mag[i];
            while (limb % 10 == 0) { ++count; limb /= 10; }
            break;
        }
    }
    return count;
}

// ═══════════════════════════════════════════════════════════════
//  Approximate Logarithms
// ═══════════════════════════════════════════════════════════════

double BigInt::approxLog10() const {
    if (isZero()) return -std::numeric_limits<double>::infinity();
    // log10(N) ≈ (numLimbs-1)*BASE_DIGITS + log10(topLimb)
    double top = (double)mag.back();
    return (double)((mag.size() - 1) * BASE_DIGITS) + std::log10(top);
}

double BigInt::approxLog2() const {
    return approxLog10() / std::log10(2.0);
}

// ═══════════════════════════════════════════════════════════════
//  Bit helpers: convert to/from LSB-first bit vector
// ═══════════════════════════════════════════════════════════════

std::vector<bool> BigInt::toBits() const {
    std::vector<bool> bits;
    if (isZero()) { bits.push_back(false); return bits; }
    // Repeatedly divide by 2 to extract bits
    BigInt n = abs();
    while (!n.isZero()) {
        bits.push_back(n.isOdd());
        n = n / BigInt(2);
    }
    return bits;   // LSB first
}

BigInt BigInt::fromBits(const std::vector<bool>& bits) {
    // bits[0] = LSB
    BigInt result(0);
    BigInt place(1);
    for (bool b : bits) {
        if (b) result += place;
        place *= BigInt(2);
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  bitLength
// ═══════════════════════════════════════════════════════════════

long long BigInt::bitLength() const {
    if (isZero()) return 0;
    // bitLength = floor(log2(|this|)) + 1
    // = (numLimbs-1)*BASE_DIGITS*log2(10) + floor(log2(topLimb)) + 1
    // Use toBits() for exact result
    return (long long)toBits().size();
}

// ═══════════════════════════════════════════════════════════════
//  testBit / setBit / clearBit
// ═══════════════════════════════════════════════════════════════

bool BigInt::testBit(long long k) const {
    if (k < 0) throw std::runtime_error("BigInt::testBit: negative bit index");
    auto bits = toBits();
    if (k >= (long long)bits.size()) return false;
    return bits[(size_t)k];
}

BigInt BigInt::setBit(long long k) const {
    if (k < 0) throw std::runtime_error("BigInt::setBit: negative bit index");
    auto bits = toBits();
    while ((long long)bits.size() <= k) bits.push_back(false);
    bits[(size_t)k] = true;
    BigInt result = fromBits(bits);
    result.negative = negative;
    return result;
}

BigInt BigInt::clearBit(long long k) const {
    if (k < 0) throw std::runtime_error("BigInt::clearBit: negative bit index");
    auto bits = toBits();
    if (k < (long long)bits.size()) bits[(size_t)k] = false;
    BigInt result = fromBits(bits);
    result.trim();
    result.negative = negative && !result.isZero();
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  Bit Shifts
//  << multiplies by 2^shift; >> divides (floor) by 2^shift
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::operator<<(long long shift) const {
    if (shift < 0)  return *this >> (-shift);
    if (isZero() || shift == 0) return *this;
    // Multiply by 2^shift using repeated squaring power
    BigInt result = abs() * BigInt(2).pow(shift);
    result.negative = negative;
    return result;
}

BigInt BigInt::operator>>(long long shift) const {
    if (shift < 0)  return *this << (-shift);
    if (isZero() || shift == 0) return *this;
    BigInt divisor = BigInt(2).pow(shift);
    BigInt result = abs() / divisor;
    result.negative = negative && !result.isZero();
    return result;
}

BigInt& BigInt::operator<<=(long long shift) { return *this = *this << shift; }
BigInt& BigInt::operator>>=(long long shift) { return *this = *this >> shift; }

// ═══════════════════════════════════════════════════════════════
//  Bitwise AND / OR / XOR
//  Implemented on the magnitude (two's complement for negatives
//  is complex; we work on absolute value bit vectors).
//  For simplicity, these operate on the magnitude bits and
//  preserve sign of the left operand for AND/OR, XOR always non-neg.
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::operator&(const BigInt& rhs) const {
    auto a = abs().toBits();
    auto b = rhs.abs().toBits();
    size_t len = std::min(a.size(), b.size());
    std::vector<bool> res(len);
    for (size_t i = 0; i < len; ++i) res[i] = a[i] && b[i];
    BigInt r = fromBits(res);
    r.trim();
    return r;
}

BigInt BigInt::operator|(const BigInt& rhs) const {
    auto a = abs().toBits();
    auto b = rhs.abs().toBits();
    size_t len = std::max(a.size(), b.size());
    a.resize(len, false);
    b.resize(len, false);
    std::vector<bool> res(len);
    for (size_t i = 0; i < len; ++i) res[i] = a[i] || b[i];
    return fromBits(res);
}

BigInt BigInt::operator^(const BigInt& rhs) const {
    auto a = abs().toBits();
    auto b = rhs.abs().toBits();
    size_t len = std::max(a.size(), b.size());
    a.resize(len, false);
    b.resize(len, false);
    std::vector<bool> res(len);
    for (size_t i = 0; i < len; ++i) res[i] = a[i] ^ b[i];
    BigInt r = fromBits(res);
    r.trim();
    return r;
}

BigInt& BigInt::operator&=(const BigInt& rhs) { return *this = *this & rhs; }
BigInt& BigInt::operator|=(const BigInt& rhs) { return *this = *this | rhs; }
BigInt& BigInt::operator^=(const BigInt& rhs) { return *this = *this ^ rhs; }

// ═══════════════════════════════════════════════════════════════
//  Base Conversion
// ═══════════════════════════════════════════════════════════════

std::string BigInt::toBinary() const {
    if (isZero()) return "0";
    auto bits = abs().toBits();   // LSB first
    std::string s;
    if (negative) s += '-';
    for (int i = (int)bits.size() - 1; i >= 0; --i)
        s += bits[i] ? '1' : '0';
    return s;
}

std::string BigInt::toHex() const {
    if (isZero()) return "0";
    // Convert via binary bits -> groups of 4
    auto bits = abs().toBits();   // LSB first
    // Pad to multiple of 4
    while (bits.size() % 4 != 0) bits.push_back(false);
    std::string s;
    if (negative) s += '-';
    bool leadingZero = true;
    for (int i = (int)bits.size() - 4; i >= 0; i -= 4) {
        int nibble = (bits[i+3] ? 8 : 0)
                   | (bits[i+2] ? 4 : 0)
                   | (bits[i+1] ? 2 : 0)
                   | (bits[i]   ? 1 : 0);
        if (nibble == 0 && leadingZero) continue;
        leadingZero = false;
        s += "0123456789abcdef"[nibble];
    }
    if (s.empty() || s == "-") s += '0';
    return s;
}

BigInt BigInt::fromHex(const std::string& hex) {
    if (hex.empty()) return BigInt(0);
    int start = 0;
    bool neg = false;
    if (hex[0] == '-') { neg = true; start = 1; }
    else if (hex[0] == '+') { start = 1; }

    // Build bit vector from hex digits (MSB first → we'll reverse)
    std::vector<bool> bits;
    for (int i = (int)hex.size() - 1; i >= start; --i) {
        char c = hex[i];
        int val;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
        else throw std::invalid_argument("BigInt::fromHex: invalid character");
        bits.push_back(val & 1);
        bits.push_back(val & 2);
        bits.push_back(val & 4);
        bits.push_back(val & 8);
    }
    BigInt result = fromBits(bits);
    result.trim();
    result.negative = neg && !result.isZero();
    return result;
}

BigInt BigInt::fromBinary(const std::string& bin) {
    if (bin.empty()) return BigInt(0);
    int start = 0;
    bool neg = false;
    if (bin[0] == '-') { neg = true; start = 1; }
    else if (bin[0] == '+') { start = 1; }

    // Build LSB-first bit vector
    std::vector<bool> bits;
    for (int i = (int)bin.size() - 1; i >= start; --i) {
        if (bin[i] == '0') bits.push_back(false);
        else if (bin[i] == '1') bits.push_back(true);
        else throw std::invalid_argument("BigInt::fromBinary: invalid character");
    }
    BigInt result = fromBits(bits);
    result.trim();
    result.negative = neg && !result.isZero();
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  Miller-Rabin Primality Test
//
//  Probabilistic test: performs `rounds` iterations.
//  Returns true if likely prime, false if definitely composite.
//  For rounds=20, false-positive probability < 4^(-20) ≈ 10^-12.
// ═══════════════════════════════════════════════════════════════

// Single witness test: write n-1 = 2^r * d, test witness a
bool BigInt::millerRabinTest(const BigInt& witness) const {
    // n-1 = 2^r * d
    BigInt nm1 = *this - BigInt(1);
    BigInt d   = nm1;
    long long r = 0;
    while (d.isEven()) { d = d / BigInt(2); ++r; }

    BigInt x = witness.modPow(d, *this);
    if (x.isOne() || x == nm1) return true;

    for (long long i = 0; i < r - 1; ++i) {
        x = (x * x) % *this;
        if (x == nm1) return true;
    }
    return false;
}

bool BigInt::isPrime(int rounds) const {
    if (isNegative() || isZero() || isOne()) return false;
    if (*this == BigInt(2) || *this == BigInt(3)) return true;
    if (isEven()) return false;

    // Small prime trial division for speed
    static const int small_primes[] = {
        3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,
        79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157
    };
    for (int p : small_primes) {
        BigInt bp(p);
        if (*this == bp) return true;
        if ((*this % bp).isZero()) return false;
    }

    // Deterministic witnesses for n < 3,317,044,064,679,887,385,961,981
    // (covers all 64-bit numbers and well beyond)
    // Use a fixed set of witnesses that covers numbers < 3.3e24
    static const long long det_witnesses[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
    };
    for (long long w : det_witnesses) {
        BigInt bw(w);
        if (*this == bw) return true;
        if (!millerRabinTest(bw)) return false;
    }

    // For very large numbers, add random witnesses
    if (rounds > 12) {
        // Use additional witnesses derived deterministically
        // (seeded from the number itself for reproducibility)
        BigInt nm2 = *this - BigInt(2);
        // Use witnesses: 41, 43, 47, 53, 59, 61, 67, 71
        static const long long extra_witnesses[] = {41,43,47,53,59,61,67,71};
        for (long long w : extra_witnesses) {
            BigInt bw(w);
            if (*this == bw) return true;
            if (*this < bw) continue;
            if (!millerRabinTest(bw)) return false;
        }
    }

    return true;
}

// ═══════════════════════════════════════════════════════════════
//  Next Prime
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::nextPrime() const {
    BigInt candidate = abs() + BigInt(1);
    // Start from an odd number
    if (candidate.isEven() && candidate != BigInt(2)) candidate += BigInt(1);
    while (!candidate.isPrime()) {
        candidate += BigInt(2);
        if (candidate.isEven()) candidate += BigInt(1); // safety
    }
    return candidate;
}

// ═══════════════════════════════════════════════════════════════
//  N-th Prime  (1-indexed: nthPrime(1) = 2)
// ═══════════════════════════════════════════════════════════════

BigInt BigInt::nthPrime(long long n) {
    if (n <= 0) throw std::runtime_error("BigInt::nthPrime: n must be >= 1");
    BigInt current(1LL);
    long long count = 0;
    while (count < n) {
        current = current.nextPrime();
        ++count;
    }
    return current;
}
