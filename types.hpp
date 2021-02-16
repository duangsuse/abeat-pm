#pragma once
#include <memory>

#define typealias_T(code) template<class T> using code<T>;
namespace abeat {
  typealias_T(Ptr = std::unique_ptr)
  typealias_T(SPtr = std::shared_ptr)
}
#undef typealias_T

#define DEFINE_LOCK(type) struct Lock { \
	explicit Lock(type *inst): o(inst) { type##_lock(inst); } \
	inline ~Lock() { if (o) type##_unlock(o); } \
	inline void unlock() { if (o) { type##_unlock(o); o = nullptr; } } \
private: type *o; };
