#include "LayerLogic.h"

#include <string.h>
#include <unistd.h>

#include "OrbitBase/Logging.h"
#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"
#include "absl/base/casts.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"

namespace {
static constexpr uint16_t kGrpcPort = 44767;
}

void LayerLogic::StartOrbitCaptureService() {
  LOG("Starting Orbit capture service");
  pid_t pid = fork();
  if (pid < 0) {
    ERROR("Fork failed; not able to start the capture service");
  } else if (pid == 0) {
    std::string game_pid = absl::StrFormat("%d", getppid());
    // TODO(crisguerrero): Read the arguments from a config file.
    char* argv[] = {strdup("/mnt/developer/OrbitCaptureGgpService"),
                    strdup("-pid"),
                    game_pid.data(),
                    strdup("-log_directory"),
                    strdup("/var/game/"),
                    NULL};

    LOG("Making call to %s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4]);
    execv(argv[0], argv);
  }
}

void LayerLogic::InitLayerData() {
  // Although this method is expected to be called just once, we include a flag to make sure the
  // gRPC service and client are not initialised more than once.
  if (data_initialised_ == false) {
    LOG("Making initialisations required in the layer");

    // Start the orbit capture service in a new thread.
    StartOrbitCaptureService();

    // Initialise the client and establish the channel to make calls to the service.
    std::string grpc_server_address = absl::StrFormat("127.0.0.1:%d", kGrpcPort);
    ggp_capture_client_ =
        std::unique_ptr<CaptureClientGgpClient>(new CaptureClientGgpClient(grpc_server_address));

    data_initialised_ = true;
  }
}

void LayerLogic::CleanLayerData() {
  if (data_initialised_ == true) {
    ggp_capture_client_->ShutdownService();
    data_initialised_ = false;
  }
}

void LayerLogic::ProcessQueuePresentKHR() {
  // TODO(crisguerrero): complete
  LOG("ProcessQueuePresentKHR called");
}