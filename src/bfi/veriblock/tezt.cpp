#include <veriblock.pb.h>
#include <veriblock.grpc.pb.h>

void foo() {
  core::Admin service;
  printf("%s\n", service.service_full_name());
}
