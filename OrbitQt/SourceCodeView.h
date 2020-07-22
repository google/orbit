#ifndef ORBIT_QT_SOURCE_CODE_VIEW_H_
#define ORBIT_QT_SOURCE_CODE_VIEW_H_

#include <QObject>

#include "WebEngine/Dialog.h"

namespace OrbitQt {

class SourceCodeView : public QObject {
  Q_OBJECT

 public:
  SourceCodeView(QWebEngineProfile* web_engine_profile,
                 QObject* parent = nullptr);

  Q_PROPERTY(QString source_code MEMBER source_code_ NOTIFY sourceCodeChanged)

  void SetSourceCode(QString new_source);
  int exec();

 signals:
  void sourceCodeChanged(const QString& new_source_code);

 private:
  WebEngine::Dialog dialog_;

  QString source_code_;
};
}  // namespace OrbitQt

#endif  // ORBIT_QT_SOURCE_CODE_VIEW_H_