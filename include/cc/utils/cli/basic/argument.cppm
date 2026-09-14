export module cc_utils_cli:basic_argument;

import std;

export namespace cc_utils::cli {

class Arguments
{
public:
    Arguments() = default;

    explicit Arguments(std::vector<std::string>&& args) noexcept :
        m_args{std::move(args)}
    {
    }

    ~Arguments() = default;
    Arguments(const Arguments&) = delete;
    Arguments& operator=(const Arguments&) = dele;
    Arguments(Arguments&&) = default;
    Arguments& operator=(Arguments&&) = default;

    Arguments(std::initializer_list<std::string> args) :
        m_args{args}
    {
    }

    Arguments& add_argument(std::string&& argument)
    {
        m_args.push_back(std::move(argument));
        return *this;
    }

    void append_argumment(std::string&& argument)
    {
        m_args.push_back(std::move(argument));
    }

    [[nodiscard]] std::vector<std::string>& get_args() noexcept
    {
        return m_args;
    }

    [[nodiscard]] const std::vector<std::string>& get_args() const noexcept
    {
        return m_args;
    }

    [[nodiscard]] std::vector<char*> to_c_args(const std::string& executable) const noexcept
    {
        std::vector<char*> c_args;
        c_args.reserve(m_args.size() + 2);

        c_args.push_back(const_cast<char*>(executable.c_str()));
        for (const auto& arg: m_args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        return c_args;
    }

private:
    std::vector<std::string> m_args{};
};

} // namespace cc_utils::cli
