#include "SourceCodeView.h"

namespace OrbitQt {

void SourceCodeView::SetSourceCode(QString new_code) {
  if (new_code != source_code_) {
    source_code_ = std::move(new_code);
    emit sourceCodeChanged(source_code_);
  }
}

int SourceCodeView::exec() {
  dialog_.show();
  return dialog_.exec();
}

SourceCodeView::SourceCodeView(QWebEngineProfile* profile, QObject* parent)
    : QObject(parent), dialog_(profile) {
  dialog_.RegisterObject("view", this);
  dialog_.GetWebEnginePage()->load(
      QUrl{"qrc:///webUI/webUI/SourceCodeView/index.html"});
  dialog_.setWindowModality(Qt::ApplicationModal);
}
}  // namespace OrbitQt