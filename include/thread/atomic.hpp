#ifndef NO_STD_THREAD_ATOMIC_HPP
#define NO_STD_THREAD_ATOMIC_HPP

namespace nostd
{
  enum MemoryOrder : int {
    RELAXED = __ATOMIC_RELAXED,
    CONSUME = __ATOMIC_CONSUME, // deprecated
    ACQUIRE = __ATOMIC_ACQUIRE,
    RELEASE = __ATOMIC_RELEASE,
    ACQ_REL = __ATOMIC_ACQ_REL, // acquire-release
    SEQ_CST = __ATOMIC_SEQ_CST  // sequential consistent
  }; // enum MemoryOrder

  template<typename T>
  class Atomic {
  private: // Fields
    T _value;

  public: // Constructors & destructors & Operators
    Atomic() noexcept = default;
    explicit Atomic(T v) noexcept : _value(v) {}
  
    Atomic(const Atomic&) = delete;
    Atomic& operator=(const Atomic&) = delete;

  public: // Methods
    T load(MemoryOrder mo = MemoryOrder::SEQ_CST) const noexcept {
      return __atomic_load_n(&_value, mo);
    }

    void store(T v, MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
      __atomic_store_n(&_value, v, mo);
    }

    T exchange(T v, MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
      return __atomic_exchange_n(&_value, v, mo);
    }
    
    bool CAS(T* expected, T desired, 
             MemoryOrder success = MemoryOrder::SEQ_CST, 
             MemoryOrder failure = MemoryOrder::ACQUIRE) noexcept {
      return __atomic_compare_exchange_n(&_value, expected, desired, false, success, failure);
    } // bool CAS

    T fetch_add(T v, MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
      return __atomic_fetch_add(&_value, v, mo);
    }

    T fetch_sub(T v, MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
      return __atomic_fetch_sub(&_value, v, mo);
    }
  }; // class Atomic

} // namespace nostd

#endif // NO_STD_THREAD_ATOMIC_HPP