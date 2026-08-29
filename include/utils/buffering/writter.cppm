export module utils_buffering:writter;

import std;
import :node;
import :reader;


export namespace utils::buffering {

class BufferWriter {
  public:
    /**
     * @brief Sets up sizing bounds — starts predicting chunks at `min_size`, never predicts
     * bigger than `max_size`. Keeps the allocator from going full send on chunk size, bet.
     * @param min_size the smallest chunk size this writer will ever predict/allocate.
     * @param max_size the largest chunk size this writer will ever predict/allocate.
     */
    explicit BufferWriter(std::size_t min_size = 8ULL * 1024ULL, std::size_t max_size = 64ULL * 1024ULL)
        : m_min_size{min_size}, m_max_size{max_size}, m_current_size{min_size} {}

    /**
     * @brief Default dtor — the underlying `BufferReader` member cleans itself up.
     */
    ~BufferWriter() = default;

    /**
     * @brief Deleted — no copying, this owns a live buffer chain.
     */
    BufferWriter(const BufferWriter &) = delete;
    /**
     * @brief Deleted, same reasoning as the copy ctor.
     */
    BufferWriter &operator=(const BufferWriter &) = delete;
    /**
     * @brief Deleted — not movable either.
     */
    BufferWriter(BufferWriter &&) = delete;
    /**
     * @brief Deleted, same reasoning as the move ctor.
     */
    BufferWriter &operator=(BufferWriter &&) = delete;

    /**
     * @brief Grabs a writable slot — reuses the current tail node if it's still got room, or
     * allocates a fresh `BufferNode` sized at the current prediction and appends it otherwise.
     * @note Whatever comes back is already acquire()'d (either by the fresh append or by the
     * explicit acquire() on the reused tail) — pair it with notify_read()'s implicit release(),
     * don't just drop it on the floor or that's a leaked reference, straight L for whoever's
     * debugging that later.
     * @return a `NodeReader` slot with room to write into.
     */
    [[nodiscard]] NodeReader *acquire() noexcept {
        auto *tail = m_view.get_tail();

        // No tail yet, or the current one's full — allocate a fresh node sized at the current
        // prediction and append it.
        if ((tail == nullptr) || tail->get_remaining() == 0) {
            // FIXME(clang-tidy): cppcoreguidelines-owning-memory — would need
            // gsl::owner<BufferNode *>, but this codebase has no GSL dependency; not a
            // mechanical fix.
            // FIXME(clang-tidy): bugprone-unhandled-exception-at-new — noexcept acquire() would
            // terminate on bad_alloc; this whole buffering subsystem has no error-return channel
            // (every push_back()/acquire() across reader/view/writter is noexcept, raw-pointer,
            // terminate-on-OOM by convention) — leaving as-is rather than inventing one locally.
            auto *node = m_view.push_back(new BufferNode{m_current_size});  // NOLINT(cppcoreguidelines-owning-memory)
            // The chain owns the NodeReader (freed on unlink) and holds the BufferNode ref its
            // ctor took. Take a SECOND BufferNode ref for the slot handed back — notify_read()/
            // release() drops exactly this one, leaving the chain's stake intact until consume().
            node->acquire();
            return node;
        }

        // Otherwise the tail's still got room — just bump its ref and reuse it. Same deal: this is
        // the caller's slot reference, dropped by notify_read()/release(); the chain keeps its own.
        tail->acquire();
        return tail;
    }

    /**
     * @brief Pushes an already-built `BufferNode` onto the chain directly, bypassing the
     * acquire()-then-write flow.
     * @param node the node to move in and append.
     */
    void push(BufferNode node) noexcept {
        // FIXME(clang-tidy): cppcoreguidelines-owning-memory — would need
        // gsl::owner<BufferNode *>, but this codebase has no GSL dependency; not a mechanical
        // fix.
        // FIXME(clang-tidy): bugprone-unhandled-exception-at-new — noexcept push() would
        // terminate on bad_alloc; same no-error-channel reasoning as acquire() above — leaving
        // as-is rather than inventing one locally.
        auto *owned_node = new BufferNode{std::move(node)};  // NOLINT(cppcoreguidelines-owning-memory)
        m_view.push_back(owned_node);
    }

    /**
     * @brief Reports back how many bytes actually landed in a slot from acquire(), folding that
     * into the reader chain's size and releasing the reference acquire() handed out. Also feeds
     * the size predictor: a full read (`bytes_read == m_current_size`) doubles the next
     * prediction (clamped), anything less shrinks the prediction down toward what actually got
     * read — no cap, it's just chasing whatever the last real read looked like.
     * @note Silently no-ops on a null `node` — safe to call even if acquire() somehow didn't hand
     * back anything usable.
     * @param node the slot previously returned by acquire().
     * @param bytes_read how many bytes actually got written into `node`.
     */
    void notify_read(NodeReader *node, std::size_t bytes_read) noexcept {
        // Safe no-op if acquire() somehow didn't hand back anything usable.
        if (node == nullptr) {
            return;
        }

        // Fold the newly-written bytes into both the node and the overall chain size, then drop
        // the reference acquire() handed out.
        node->expand_written(bytes_read);
        m_view.expand(bytes_read);
        node->release();

        // Chase whatever the last real read looked like: filled the slot completely? Double the
        // next prediction (clamped). Came up short? Shrink the prediction down toward it.
        if (bytes_read == m_current_size) {
            m_current_size = std::clamp(m_current_size * 2, m_min_size, m_max_size);
        } else {
            m_current_size = std::clamp(bytes_read, m_min_size, m_max_size);
        }
    }

    /**
     * @brief Drops the reference acquire() handed out for a slot that ended up carrying no read
     * (would-block, error). Unlike notify_read() it folds nothing into the chain and leaves the
     * size predictor untouched — nothing was actually read, so the allocation sizing must not
     * move.
     * @note No-ops on a null node, same as notify_read().
     * @param node the slot previously returned by acquire().
     */
    void release(NodeReader *node) noexcept {
        if (node == nullptr) {
            return;
        }
        node->release();
    }

    /**
     * @brief Grabs the current size prediction for the next allocated chunk.
     * @return the predicted next chunk size, in bytes.
     */
    [[nodiscard]] std::size_t get_predicted_size() const noexcept { return m_current_size; }
    /**
     * @brief Grabs the underlying reader chain this writer feeds into.
     * @return a mutable reference to the backing `BufferReader`.
     */
    [[nodiscard]] BufferReader &get_view() noexcept { return m_view; }

  private:
    std::size_t m_min_size;
    std::size_t m_max_size;
    std::size_t m_current_size;
    BufferReader m_view;
};


} // namespace utils::buffering
