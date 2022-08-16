// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-spec-builders.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <tuple>

#include "MetricsUploader/MockMetricsUploader.h"
#include "MetricsUploader/SymbolLoadingMetric.h"
#include "MetricsUploader/orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

using ::testing::_;

namespace {

void OrbitPerModuleSymbolLoadDataAreEqual(const OrbitPerModuleSymbolLoadData& lhs,
                                          const OrbitPerModuleSymbolLoadData& rhs) {
  EXPECT_EQ(lhs.symbols_found(), rhs.symbols_found());
  EXPECT_EQ(lhs.symbol_file_separation(), rhs.symbol_file_separation());
  EXPECT_EQ(lhs.is_main_module(), rhs.is_main_module());
  EXPECT_EQ(lhs.symbol_source(), rhs.symbol_source());
}

}  // namespace

class SymbolLoadingMetricTest : public testing::Test {
 public:
  void ExpectSendSymbolLoadEvent(const OrbitPerModuleSymbolLoadData& symbol_load_data) {
    ExpectSendSymbolLoadEvent(symbol_load_data, OrbitLogEvent::SUCCESS);
  }
  void ExpectSendSymbolLoadEvent(const OrbitPerModuleSymbolLoadData& symbol_load_data,
                                 OrbitLogEvent::StatusCode status_code) {
    ExpectSendSymbolLoadEvent(symbol_load_data, std::chrono::milliseconds{0}, status_code);
  }
  void ExpectSendSymbolLoadEvent(const OrbitPerModuleSymbolLoadData& symbol_load_data,
                                 std::chrono::milliseconds event_duration,
                                 OrbitLogEvent::StatusCode status_code) {
    EXPECT_CALL(uploader_, SendSymbolLoadEvent(_, _, _))
        .Times(1)
        .WillOnce([=](const OrbitPerModuleSymbolLoadData& call_symbol_load_data,
                      std::chrono::milliseconds call_event_duration,
                      OrbitLogEvent::StatusCode call_status_code) -> bool {
          EXPECT_EQ(status_code, call_status_code);
          EXPECT_GE(call_event_duration, event_duration);
          OrbitPerModuleSymbolLoadDataAreEqual(symbol_load_data, call_symbol_load_data);

          return true;
        });
  }

 protected:
  MockMetricsUploader uploader_;
};

TEST_F(SymbolLoadingMetricTest, Move) {
  ExpectSendSymbolLoadEvent(OrbitPerModuleSymbolLoadData{});

  SymbolLoadingMetric metric{&uploader_};
  const SymbolLoadingMetric moved_metric{std::move(metric)};
}

TEST_F(SymbolLoadingMetricTest, Delay) {
  SymbolLoadingMetric metric{&uploader_};

  constexpr std::chrono::milliseconds kDelay{5};
  const OrbitPerModuleSymbolLoadData kDefaultSymbolLoadData;

  ExpectSendSymbolLoadEvent(kDefaultSymbolLoadData, kDelay, OrbitLogEvent::SUCCESS);

  std::this_thread::sleep_for(kDelay);
}

TEST_F(SymbolLoadingMetricTest, SetIsMainModuleFalse) {
  OrbitPerModuleSymbolLoadData symbol_load_data;
  symbol_load_data.set_is_main_module(OrbitPerModuleSymbolLoadData::IS_MAIN_MODULE_FALSE);

  ExpectSendSymbolLoadEvent(symbol_load_data);

  SymbolLoadingMetric metric{&uploader_};
  metric.SetIsMainModule(false);
}

TEST_F(SymbolLoadingMetricTest, SetIsMainModuleTrue) {
  OrbitPerModuleSymbolLoadData symbol_load_data;
  symbol_load_data.set_is_main_module(OrbitPerModuleSymbolLoadData::IS_MAIN_MODULE_TRUE);

  ExpectSendSymbolLoadEvent(symbol_load_data);

  SymbolLoadingMetric metric{&uploader_};
  metric.SetIsMainModule(true);
}

TEST_F(SymbolLoadingMetricTest, SetSymbolsNotFound) {
  OrbitPerModuleSymbolLoadData symbol_load_data;
  symbol_load_data.set_symbols_found(OrbitPerModuleSymbolLoadData::SYMBOLS_FOUND_FALSE);

  ExpectSendSymbolLoadEvent(symbol_load_data);

  SymbolLoadingMetric metric{&uploader_};
  metric.SetSymbolsNotFound();
}

TEST_F(SymbolLoadingMetricTest, SetSymbolsFound) {
  OrbitPerModuleSymbolLoadData symbol_load_data;
  symbol_load_data.set_symbols_found(OrbitPerModuleSymbolLoadData::SYMBOLS_FOUND_TRUE);
  symbol_load_data.set_symbol_file_separation(OrbitPerModuleSymbolLoadData::DIFFERENT_FILE);
  symbol_load_data.set_symbol_source(OrbitPerModuleSymbolLoadData::ORBIT_CACHE);

  ExpectSendSymbolLoadEvent(symbol_load_data);

  SymbolLoadingMetric metric{&uploader_};
  metric.SetSymbolsFound(OrbitPerModuleSymbolLoadData::ORBIT_CACHE,
                         OrbitPerModuleSymbolLoadData::DIFFERENT_FILE);
}

TEST_F(SymbolLoadingMetricTest, SetError) {
  OrbitPerModuleSymbolLoadData symbol_load_data;

  ExpectSendSymbolLoadEvent(symbol_load_data, OrbitLogEvent::INTERNAL_ERROR);

  SymbolLoadingMetric metric{&uploader_};
  metric.SetError();
}

TEST_F(SymbolLoadingMetricTest, SetCancelled) {
  OrbitPerModuleSymbolLoadData symbol_load_data;

  ExpectSendSymbolLoadEvent(symbol_load_data, OrbitLogEvent::CANCELLED);

  SymbolLoadingMetric metric{&uploader_};
  metric.SetCancelled();
}

};  // namespace orbit_metrics_uploader