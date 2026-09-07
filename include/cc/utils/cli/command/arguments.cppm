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

    Arguments& add(std::string&& argument)
    {
        m_args.push_back(std::move(argument));
        return *this;
    }

    Arguments& add(const std::string& argument)
    {
        m_args.push_back(argument);
        return *this;
    }

    Arguments& add(std::initializer_list<std::string> arguments)
    {
        m_args.insert(m_args.end(), arguments.begin(), arguments.end());
        return *this;
    }

    const std::vector<std::string>& get_args() const
    {
        return m_args;
    }

    std::vector<char*> get_c_args(const std::string& executable) const
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
