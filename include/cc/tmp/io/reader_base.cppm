module;

#include <string>
#include <memory>

export module cc_tmp:io_reader_base;

import std;
import cc_abi;

export {

namespace tensorflow {

class ReaderBase {
public:
    ReaderBase() : m_file{} {}
    virtual ~ReaderBase() = default;

    virtual ice::Status Read(std::string* key, std::string* value) = 0;
    virtual ice::Status Reset() = 0;

private:
    ice::RandomAccessFile m_file;
};

} // namespace tensorflow

} // export
