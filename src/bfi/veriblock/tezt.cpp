#include <veriblock.pb.h>
#include <veriblock.grpc.pb.h>

int main (){
  core::Admin service;
  printf("%s\n", service.service_full_name());
}
