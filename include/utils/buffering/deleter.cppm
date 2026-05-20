export module utils_buffering:deleter;

import std;

export namespace utils::buffering {

class Deleter {
  public:
    struct Internal {
        int m_ref_count{1};
        virtual void destroy() noexcept = 0;
        virtual ~Internal() = default;
    };

    template <std::invocable Action>
    struct ConcreteInternal final : Internal {
        Action m_action;
        explicit ConcreteInternal(Action &&a) : m_action(std::move(a)) {}
        void destroy() noexcept override { m_action(); }
    };

    Deleter() : m_internal{nullptr} {}

    template <std::invocable Action>
    explicit Deleter(Action &&action) : m_internal(new ConcreteInternal<Action>(std::forward<Action>(action))) {}

    Deleter(const Deleter &other) noexcept : m_internal(other.m_internal) {
        if (m_internal != nullptr) {
            ++m_internal->m_ref_count;
        }
    }

    Deleter(Deleter &&other) noexcept : m_internal(std::exchange(other.m_internal, nullptr)) {}

    Deleter &operator=(Deleter other) noexcept {
        std::swap(m_internal, other.m_internal);
        return *this;
    }

    ~Deleter() noexcept { release(); }

    void release() noexcept {
        if ((m_internal != nullptr) && --m_internal->m_ref_count == 0) {
            m_internal->destroy();
            delete m_internal;
        }
    }

    [[nodiscard]] bool empty() const noexcept { return m_internal == nullptr; }
    [[nodiscard]] int use_count() const noexcept { return (m_internal != nullptr) ? m_internal->m_ref_count : 0; }

  private:
    Internal *m_internal;
};

} // namespace utils::buffering
