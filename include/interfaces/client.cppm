export module interfaces:client;

import io_shared;
import shared;
import :io;

export namespace interfaces {

class IClient {
  public:
    virtual ~IClient() = default;

    // Disallow copying to ensure clean polymorphic handling
    IClient(const IClient &) = delete;
    IClient &operator=(const IClient &) = delete;

    IClient(IClient &&) noexcept = default;
    IClient &operator=(IClient &&) noexcept = default;

    virtual shared::ReadCallback on_connect(shared::SendCallback send,
                                            shared::CloseCallback close) = 0;

    virtual void send(io::IRequest &request) = 0;

  protected:
    IClient() = default;
};

} // namespace interfaces
