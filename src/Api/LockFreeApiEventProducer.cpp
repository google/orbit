#include "LockFreeApiEventProducer.h"

namespace orbit_api {

orbit_grpc_protos::ProducerCaptureEvent* LockFreeApiEventProducer::TranslateIntermediateEvent(
    ApiEventVariant&& raw_api_event, google::protobuf::Arena* arena) {
  auto* capture_event =
      google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);

  std::visit(
      [capture_event](const auto& event) {
        orbit_api::FillProducerCaptureEventFromApiEvent(event, capture_event);
      },
      raw_api_event);

  return capture_event;
}

}  // namespace orbit_api