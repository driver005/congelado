module core.server.server;
@nogc nothrow:

import interfaces.request  : IRequest;
import interfaces.response : IResponse;
import core.server.types   : Method;
import core.server.router  : RouteHandler;

class Server(Derived) {
  public:
    this(RouteHandler!Derived handler) {
        m_handler = handler;
    }

    void match(Method method, const(char)[] path,
               ref IRequest!Derived req, ref IResponse!Derived res) {
        m_handler.match(method, path, req, res);
    }

  private:
    RouteHandler!Derived m_handler;
}
