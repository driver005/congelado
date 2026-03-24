export module qpack;

export import :table;

export namespace codec::qpack {

template <std::unsigned_integral UInt = std::uint32_t, int Width = 4>
    requires shared::DecodeWidth<Width>
class QPack {};
} // namespace codec::qpack
