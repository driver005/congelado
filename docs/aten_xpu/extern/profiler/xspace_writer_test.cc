#include "docs/aten_xpu/extern/profiler/proto/xplane.pb.h"
#include "docs/aten_xpu/extern/profiler/xspace_writer.h"

#include <cstdio>
#include <string>

namespace {

int g_failures = 0;

void expect(bool condition, const char* what)
{
    if (!condition) {
        std::fprintf(stderr, "FAILED: %s\n", what);
        ++g_failures;
    }
}

} // namespace

int main()
{
    aten_xpu::XSpaceWriter writer;
    const auto plane = writer.add_plane("/device:XPU:0", 0);
    const auto copy_line = writer.add_line(plane, "copy", 1'000);
    const auto compute_line = writer.add_line(plane, "compute", 2'000);
    writer.add_event(plane, copy_line, "memcpy_htod", 1'500, 250, 4'096);
    writer.add_event(plane, copy_line, "memcpy_htod", 2'000, 100, 0);
    writer.add_event(plane, compute_line, "matmul", 2'100, 900, 0);
    writer.add_warning("device has no queue_profiling aspect");
    writer.add_hostname("host-a");

    tensorflow::profiler::XSpace space;
    expect(space.ParseFromString(writer.serialize()), "serialised bytes parse as XSpace");
    expect(space.planes_size() == 1, "one plane");
    expect(space.warnings_size() == 1 && space.hostnames_size() == 1, "warning and hostname");

    const auto& parsed = space.planes(0);
    expect(parsed.name() == "/device:XPU:0", "plane name");
    expect(parsed.lines_size() == 2, "two lines");
    expect(parsed.event_metadata_size() == 2, "event names are interned (memcpy_htod, matmul)");
    expect(parsed.stat_metadata().at(1).name() == "bytes", "bytes stat metadata");

    const auto& copy = parsed.lines(0);
    expect(copy.name() == "copy" && copy.timestamp_ns() == 1'000, "copy line header");
    expect(copy.events_size() == 2, "two copy events");
    expect(copy.events(0).offset_ps() == 500'000, "offset is relative to the line, in ps");
    expect(copy.events(0).duration_ps() == 250'000, "duration in ps");
    expect(
        copy.events(0).stats_size() == 1 && copy.events(0).stats(0).uint64_value() == 4'096,
        "bytes stat"
    );
    expect(copy.events(1).stats_size() == 0, "no stat when bytes is zero");
    expect(copy.events(0).metadata_id() == copy.events(1).metadata_id(), "same name, same id");
    expect(copy.duration_ps() == 1'100'000, "line duration is the latest event end");

    if (g_failures != 0) {
        return 1;
    }
    std::puts("xspace_writer_test: ok");

    return 0;
}
