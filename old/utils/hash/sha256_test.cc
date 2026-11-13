import std;
import utils_hash;

namespace {

class Check
{
public:
    void equal(std::string_view actual, std::string_view expected, std::string_view label)
    {
        if (actual != expected) {
            std::println(std::cerr, "FAIL {}: expected '{}', got '{}'", label, expected, actual);
            ++m_failures;
        }
    }

    void yes(bool condition, std::string_view label)
    {
        if (!condition) {
            std::println(std::cerr, "FAIL {}", label);
            ++m_failures;
        }
    }

    int failures() const
    {
        return m_failures;
    }

private:
    int m_failures{0};
};

} // namespace

int main()
{
    Check check;

    check.equal(
        utils::Sha256::hash_hex(""),
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        "empty input"
    );
    check.equal(
        utils::Sha256::hash_hex("abc"),
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
        "abc"
    );

    std::string digest = utils::Sha256::hash_hex("congelado");
    check.yes(digest.size() == 64, "digest is 64 characters");
    check.yes(digest == utils::Sha256::hash_hex("congelado"), "digest is deterministic");
    check.yes(
        std::ranges::all_of(
            digest,
            [](char character)
            {
                return (character >= '0' && character <= '9') ||
                       (character >= 'a' && character <= 'f');
            }
        ),
        "digest is lowercase hex"
    );

    return check.failures() == 0 ? 0 : 1;
}
