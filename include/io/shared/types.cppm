export module io_shared:types;

import std;

export namespace io::shared {

enum class Role : std::uint8_t { SENDER = 0, RECEIVER = 1 };

} // namespace io::shared
