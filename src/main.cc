#include "congelado.h"

class ProcessIMPL : public expcmake::RouteGuide::Service {
  ::grpc::Status GetFeature(::grpc::ServerContext *,
                            const ::expcmake::Address *,
                            ::expcmake::Address *response) {
    fmt::println("called");
    response->set_name("test");
    return grpc::Status::OK;
  }
};

int main() {
  fmt::print(fg(fmt::terminal_color::cyan), "Hello fmt {}!\n", FMT_VERSION);

  ProcessIMPL service;
  grpc::ServerBuilder builder;

  builder.AddListeningPort("0.0.0.0:9999", grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

  server->Wait();

  return 0;
}