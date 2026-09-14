export module cc_utils_cli:arguments;

import std;

export namespace cc_utils::cli {

class Arguments
{
public:
    Arguments() = default;

    explicit Arguments(std::vector<std::string>&& args) :
        m_args{std::move(args)}
    {
    }

    Arguments(std::initializer_list<std::string> args) :
        m_args{args}
    {
    }

    ~Arguments() = default;
    Arguments(const Arguments&) = delete;
    Arguments& operator=(const Arguments&) = delete;
    Arguments(Arguments&&) = default;
    Arguments& operator=(Arguments&&) = default;

    Arguments& add_arg(std::string&& argument) noexcept
    {
        m_args.push_back(std::move(argument));
        return *this;
    }

    Arguments& add_arg(std::initializer_list<std::string> arguments) noexcept
    {
        m_args.insert(m_args.end(), arguments.begin(), arguments.end());
        return *this;
    }

    void append_arg(std::string&& argument) noexcept
    {
        m_args.push_back(std::move(argument));
    }

    std::span<const std::string> get_args() const noexcept
    {
        return m_args;
    }

    std::vector<char*> to_c_args(const std::string& executable) const noexcept
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
    std::vector<std::string> m_args;
};

} // namespace cc_utils::cli
