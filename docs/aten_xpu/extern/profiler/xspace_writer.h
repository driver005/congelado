#ifndef ATEN_XPU_EXTERN_PROFILER_XSPACE_WRITER_H_
#define ATEN_XPU_EXTERN_PROFILER_XSPACE_WRITER_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace aten_xpu {

// Builds a TensorFlow XSpace (tensorflow.profiler) and serialises it. Protobuf stays behind this
// header so modules can use it without including generated code.
class XSpaceWriter
{
public:
    XSpaceWriter();
    ~XSpaceWriter();

    XSpaceWriter(const XSpaceWriter&) = delete;
    XSpaceWriter& operator=(const XSpaceWriter&) = delete;
    XSpaceWriter(XSpaceWriter&&) = delete;
    XSpaceWriter& operator=(XSpaceWriter&&) = delete;

    // Returns the plane index. Example name: /device:XPU:0
    std::size_t add_plane(std::string_view name, std::int64_t id);

    // Returns the line index inside `plane`. `timestamp_ns` is the start of the line.
    std::size_t add_line(std::size_t plane, std::string_view name, std::int64_t timestamp_ns);

    // `start_ns` is absolute; the stored offset is relative to the line's timestamp.
    void add_event(
        std::size_t plane,
        std::size_t line,
        std::string_view name,
        std::int64_t start_ns,
        std::int64_t duration_ns,
        std::uint64_t bytes
    );

    void add_warning(std::string_view text);
    void add_hostname(std::string_view name);

    [[nodiscard]] std::string serialize() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace aten_xpu

#endif // ATEN_XPU_EXTERN_PROFILER_XSPACE_WRITER_H_
