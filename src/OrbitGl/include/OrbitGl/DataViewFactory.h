// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DATA_VIEW_FACTORY_H_
#define ORBIT_GL_DATA_VIEW_FACTORY_H_

#include <memory>

#include "DataViews/DataView.h"

// Interface to DataView factory.
// This class establishes interface to data view factory
class DataViewFactory {
 public:
  DataViewFactory() = default;
  virtual ~DataViewFactory() = default;

  // Creates DataView of specified type. The created data view
  // exists in application scope. The implementation will create
  // a DataView of specified type or return exiting one. The assumption
  // here is that there is only one of each type of data view needed.
  //
  // Note that SamplingReportDataView should not be created using this method
  // since it is owned and created by SamplingReport.
  virtual orbit_data_views::DataView* GetOrCreateDataView(orbit_data_views::DataViewType type) = 0;
};

#endif  // ORBIT_GL_DATA_VIEW_FACTORY_H_
