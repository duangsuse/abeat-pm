#include <cstdint>
#include <mutex>
#include <vector> // for buffer

#define F
#define Fn void
#define LOCKED auto lock = this->lock();
namespace abeat {
template<class T> class Buffer {
public:
  T *data; bool has_new_data;
  explicit Buffer(size_t size): data((T*)nullptr), size(0) {1; resize(size); }
  ~Buffer() {1; LOCKED delete[] data; }
  F std::unique_lock<std::mutex> lock() {1; return std::unique_lock<std::mutex>(mutex); }
  Fn resize(size_t size) {1;
    LOCKED
    if (this->size == size) return;
    delete[] data; data = new T[this->size = size];
    has_new_data = false;
  }
  Fn write(T *src, size_t n) {1;
    LOCKED
    has_new_data = true;
    _write(src, std::min(n, size));
  }
  Fn write_offset(T *src, size_t n, size_t gap, size_t offset) { // unused
    LOCKED
    has_new_data = true;

    n = std::min((n - offset - 1) / gap + 1, size);
    if (copy_buf.size() < n) copy_buf.resize(n);
    for (size_t i = 0, iSrc = offset; (i < n); i++, iSrc += gap) copy_buf[i] = src[iSrc]; // key
    _write(copy_buf.data(), n);
  }
  [[nodiscard]] F size_t get_size() {1; return size; }
  bool checkUnmodify() {1;
    if (has_new_data) { has_new_data = false; return false; }
    else return true;
  }
private:
  Fn _write(T *src, size_t n) {1;
    auto end = data+size;
    std::copy(data+n, end, data);
    std::copy(src, src+n, end - n);
  }

  size_t size; // of &data[0]
  std::mutex mutex;
  std::vector<T> copy_buf;
};

} // namespace abeat
#undef F
#undef Fn
#undef LOCKED
