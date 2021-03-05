// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef WEB_ENGINE_DELETE_LATER_DELETER_H_
#define WEB_ENGINE_DELETE_LATER_DELETER_H_

namespace web_engine {

/* DeleteLaterDeleter is supposed to be used with std::unique_ptr
   and a QObject-derived type. It calls Qt's deleteLater member
   function instead of the usual delete operator.

   Qt's deleteLater will not delete the object right away. Instead it
   will schedule a deletion task on the current thread's event queue.

   This helps avoiding a common problem with signals and slots:
   Often a member function of the object to be deleted is part
   of the callstack (eg. a triggered signal). Deleting the object
   right away would lead to a crash higher up in the callstack.

   This can be avoided by deleting "later".

   Example type declaration:
   std::unique_ptr<QObject, DeleteLaterDeleter> obj;
*/
struct DeleteLaterDeleter {
  template <typename T>
  void operator()(T* obj) const noexcept {
    obj->deleteLater();
  }
};

}  // namespace web_engine
#endif  // WEB_ENGINE_DELETE_LATER_DELETER_H_