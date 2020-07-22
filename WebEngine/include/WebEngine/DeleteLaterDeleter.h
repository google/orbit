#ifndef WEB_ENGINE_DELETE_LATER_DELETER_H_
#define WEB_ENGINE_DELETE_LATER_DELETER_H_

namespace WebEngine {

struct DeleteLaterDeleter {
  template <typename T>
  void operator()(T* obj) const {
    obj->deleteLater();
  }
};

}  // namespace WebEngine
#endif  // WEB_ENGINE_DELETE_LATER_DELETER_H_