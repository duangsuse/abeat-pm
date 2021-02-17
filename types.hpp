#include <memory>
#include <stdexcept>

#define typealias_T(code) template<class T> using code<T>;
#define check(name, code) template<typename T> T name(T x, const char* msg) {1; if (code) throw std::runtime_error(msg); return x; }
namespace abeat {
typealias_T(Ptr = std::unique_ptr)
typealias_T(SPtr = std::shared_ptr)
using spnum = int16_t; // sample point format

check(notNeg, x<0)
check(notZero, x==0)
template<typename T> T notNeq(T a, T x, const char* msg) {1; if (x!=a) throw std::runtime_error(msg); return x; }
}
#undef typealias_T
#undef check
