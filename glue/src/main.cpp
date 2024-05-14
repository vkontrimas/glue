#include <glog/logging.h>

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;

  LOG(INFO) << "Hello world";

  return 0;
}