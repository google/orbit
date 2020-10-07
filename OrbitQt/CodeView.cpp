// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeView.h"

#include <QSize>

#include "OrbitBase/Logging.h"

namespace orbit_qt {

void CodeView::SetCode(QString title, QString new_code, QString language) {
  title_ = std::move(title);
  source_code_ = std::move(new_code);
  language_ = std::move(language);
  emit sourceCodeChanged();
}

void CodeView::SetCode(QString title, QString new_code, QString language,
                       DisassemblyReport disassembly_report) {
  disassembly_report_ = std::move(disassembly_report);
  SetCode(std::move(title), std::move(new_code), std::move(language));
}

QVector<int> CodeView::GetHitCountsPerLine() const {
  if (!disassembly_report_) {
    return {};
  }

  const auto number_of_code_lines = source_code_.count('\n');
  QVector<int> hit_counts;
  hit_counts.reserve(number_of_code_lines);

  for (int i = 0; i < number_of_code_lines; i++) {
    const auto samples = static_cast<int>(disassembly_report_->GetNumSamplesAtLine(i));
    hit_counts.push_back(samples);
  }

  return hit_counts;
}

QVector<double> CodeView::GetHitRatiosPerLine() const {
  if (!disassembly_report_) {
    return {};
  }

  const auto number_of_code_lines = source_code_.count('\n');
  QVector<double> hit_ratios;
  hit_ratios.reserve(number_of_code_lines);

  const auto number_of_samples_in_function = disassembly_report_->GetNumSamplesInFunction();

  for (int i = 0; i < number_of_code_lines; i++) {
    const auto samples = static_cast<int>(disassembly_report_->GetNumSamplesAtLine(i));
    hit_ratios.push_back(static_cast<double>(samples) /
                         static_cast<double>(number_of_samples_in_function));
  }

  return hit_ratios;
}

QVector<double> CodeView::GetTotalHitRatiosPerLine() const {
  if (!disassembly_report_) {
    return {};
  }

  const auto number_of_code_lines = source_code_.count('\n');
  QVector<double> total_hit_ratios;
  total_hit_ratios.reserve(number_of_code_lines);

  const auto total_number_of_samples = disassembly_report_->GetNumSamples();

  for (int i = 0; i < number_of_code_lines; i++) {
    const auto samples = static_cast<int>(disassembly_report_->GetNumSamplesAtLine(i));
    total_hit_ratios.push_back(static_cast<double>(samples) /
                               static_cast<double>(total_number_of_samples));
  }

  return total_hit_ratios;
}

CodeView::CodeView(std::optional<int> web_socket_port, QObject* parent)
    : QObject(parent), web_engine_view_(web_socket_port) {
  web_engine_view_.RegisterObject("view", this);

  if (const auto server = web_engine_view_.GetWebSocketServer(); server) {
    const auto listen_port = server.value()->serverPort();
    LOG("CodeView's websocket server is listening on port %d.", listen_port);
  }
}
}  // namespace orbit_qt