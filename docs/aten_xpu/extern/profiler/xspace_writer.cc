#include "docs/aten_xpu/extern/profiler/xspace_writer.h"

#include "docs/aten_xpu/extern/profiler/proto/xplane.pb.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace aten_xpu {

namespace {

    constexpr std::int64_t PICOSECONDS_PER_NANOSECOND = 1'000;
    constexpr std::int64_t BYTES_STAT_ID = 1;

} // namespace

// Interned event names per plane: the event metadata table of an XPlane maps ids to names.
class XSpaceWriter::Impl
{
public:
    tensorflow::profiler::XSpace m_space;
    std::vector<std::unordered_map<std::string, std::int64_t>> m_event_ids;
};

XSpaceWriter::XSpaceWriter() :
    m_impl{std::make_unique<Impl>()}
{
}

XSpaceWriter::~XSpaceWriter() = default;

std::size_t XSpaceWriter::add_plane(std::string_view name, std::int64_t id)
{
    auto* plane = m_impl->m_space.add_planes();
    plane->set_id(id);
    plane->set_name(std::string{name});

    auto& stat = (*plane->mutable_stat_metadata())[BYTES_STAT_ID];
    stat.set_id(BYTES_STAT_ID);
    stat.set_name("bytes");

    m_impl->m_event_ids.emplace_back();

    return static_cast<std::size_t>(m_impl->m_space.planes_size() - 1);
}

std::size_t
XSpaceWriter::add_line(std::size_t plane, std::string_view name, std::int64_t timestamp_ns)
{
    auto* target = m_impl->m_space.mutable_planes(static_cast<int>(plane));
    auto* line = target->add_lines();
    line->set_id(target->lines_size());
    line->set_name(std::string{name});
    line->set_timestamp_ns(timestamp_ns);

    return static_cast<std::size_t>(target->lines_size() - 1);
}

void XSpaceWriter::add_event(
    std::size_t plane,
    std::size_t line,
    std::string_view name,
    std::int64_t start_ns,
    std::int64_t duration_ns,
    std::uint64_t bytes
)
{
    auto* target = m_impl->m_space.mutable_planes(static_cast<int>(plane));
    auto& ids = m_impl->m_event_ids.at(plane);

    const std::string key{name};
    auto found = ids.find(key);
    if (found == ids.end()) {
        const auto id = static_cast<std::int64_t>(ids.size()) + 1;
        auto& metadata = (*target->mutable_event_metadata())[id];
        metadata.set_id(id);
        metadata.set_name(key);
        found = ids.emplace(key, id).first;
    }

    auto* target_line = target->mutable_lines(static_cast<int>(line));
    auto* event = target_line->add_events();
    event->set_metadata_id(found->second);
    event->set_offset_ps((start_ns - target_line->timestamp_ns()) * PICOSECONDS_PER_NANOSECOND);
    event->set_duration_ps(duration_ns * PICOSECONDS_PER_NANOSECOND);
    if (bytes != 0) {
        auto* stat = event->add_stats();
        stat->set_metadata_id(BYTES_STAT_ID);
        stat->set_uint64_value(bytes);
    }

    const auto end_ps = event->offset_ps() + event->duration_ps();
    target_line->set_duration_ps(std::max(target_line->duration_ps(), end_ps));
}

void XSpaceWriter::add_warning(std::string_view text)
{
    m_impl->m_space.add_warnings(std::string{text});
}

void XSpaceWriter::add_hostname(std::string_view name)
{
    m_impl->m_space.add_hostnames(std::string{name});
}

std::string XSpaceWriter::serialize() const
{
    return m_impl->m_space.SerializeAsString();
}

} // namespace aten_xpu
