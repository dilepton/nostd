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

  inline void thread_fence(MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
    __atomic_thread_fence(mo);
  }

  template<typename T>
  class Atomic {
  private: // Assertions & Usings & Constants
    static_assert(__is_trivially_copyable(T), "Atomic<T>: T must be trivially copyable");
    static_assert(__atomic_always_lock_free(sizeof(T), 0), "Atomic<T>: not lock-free on this target, would need libatomic");

  private: // Fields
    alignas(sizeof(T)) T _value{};

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

  template<typename T>
  class Atomic<T&> {
  private: // Assertions & Usings & Constants
    static_assert(__is_trivially_copyable(T), "Atomic<T>: T must be trivially copyable");
    static_assert(__atomic_always_lock_free(sizeof(T), 0), "Atomic<T>: not lock-free on this target, would need libatomic");

  private: // Fields
    T* _p;

  public: // Constructors & destructors & Operators
    Atomic() = delete;
    explicit Atomic(T& ref) noexcept : _p(&ref) {}
    explicit Atomic(T* ptr) noexcept : _p(ptr) {
      __builtin_assume(reinterpret_cast<unsigned long>(ptr) % alignof(T) == 0);
    }

    Atomic(const Atomic&) noexcept = default;
    Atomic& operator=(const Atomic&) noexcept = default;

  public: // Methods
    T load(MemoryOrder mo = MemoryOrder::SEQ_CST) const noexcept {
      return __atomic_load_n(_p, mo);
    }

    void store(T v, MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
      __atomic_store_n(_p, v, mo);
    }

    T exchange(T v, MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
      return __atomic_exchange_n(_p, v, mo);
    }

    bool CAS(T* expected, T desired,
             MemoryOrder s = MemoryOrder::SEQ_CST, MemoryOrder f = MemoryOrder::ACQUIRE) noexcept {
      return __atomic_compare_exchange_n(_p, expected, desired, false, s, f);
    } // bool CAS

    T fetch_add(T v, MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
      return __atomic_fetch_add(_p, v, mo);
    }

    T fetch_sub(T v, MemoryOrder mo = MemoryOrder::SEQ_CST) noexcept {
      return __atomic_fetch_sub(_p, v, mo);
    }

  }; // class Atomic<T&>

} // namespace nostd

#endif // NO_STD_THREAD_ATOMIC_HPP