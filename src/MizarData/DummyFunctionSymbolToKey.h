// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_DUMMY_FUNCTION_SYMBOL_TO_KEY_H_
#define MIZAR_DATA_DUMMY_FUNCTION_SYMBOL_TO_KEY_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <string>

#include "MizarBase/FunctionSymbols.h"

namespace orbit_mizar_data_internal {
static const auto* kDirectXToDxvkNames = new absl::flat_hash_map<std::string, std::string>{
    {"CContext::TID3D11DeviceContext_ClearRenderTargetView_<1>",
     "dxvk::D3D11DeviceContext::ClearRenderTargetView(ID3D11RenderTargetView*, float const*)"},
    {"CContext::TID3D11DeviceContext_SetShader_<1,0>",
     "dxvk::D3D11DeviceContext::PSSetShader(ID3D11PixelShader*, ID3D11ClassInstance* const*, "
     "unsigned int)"},
    {"CContext::TID3D11DeviceContext_Draw_<9>(ID3D11DeviceContext5 *,unsigned int,unsigned int)",
     "dxvk::D3D11DeviceContext::Draw(unsigned int, unsigned int)"},
    {"CContext::TID3D11DeviceContext_SetShader_<1,4>",
     "dxvk::D3D11DeviceContext::VSSetShader(ID3D11VertexShader*, ID3D11ClassInstance* const*, "
     "unsigned int)"},
    {"CDXGISwapChain::Present",
     "dxvk::D3D11SwapChain::Present(unsigned int, unsigned int, DXGI_PRESENT_PARAMETERS const*)"}};

static const auto* kMappableModules = new absl::flat_hash_set<std::string>{"d3d11", "dxgi"};

}  // namespace orbit_mizar_data_internal

namespace orbit_mizar_data {

// If a function comes from `mappableModules`, the key from `functionNameToKey` is returned if
// present. Defaulting to the function name as the key.
template <auto& functionNameToKey, auto& mappableModules>
class DummyFunctionSymbolToKey {
  using FunctionSymbol = ::orbit_mizar_base::FunctionSymbol;

 public:
  [[nodiscard]] std::string GetKey(const FunctionSymbol& symbol) const {
    if (mappableModules->contains(symbol.module_file_name)) {
      if (const auto it = functionNameToKey->find(symbol.function_name);
          it != functionNameToKey->end()) {
        return it->second;
      }
    }
    return symbol.function_name;
  }
};

using D3D11DummyFunctionSymbolToKey =
    DummyFunctionSymbolToKey<orbit_mizar_data_internal::kDirectXToDxvkNames,
                             orbit_mizar_data_internal::kMappableModules>;

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_DUMMY_FUNCTION_SYMBOL_TO_KEY_H_
