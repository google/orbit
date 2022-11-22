// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DummyFunctionSymbolToKey.h"

#include <string>
#include <utility>

#include "MizarBase/FunctionSymbols.h"

using ::orbit_mizar_base::FunctionSymbol;

namespace orbit_mizar_data {

std::string DummyFunctionSymbolToKey::GetKey(const FunctionSymbol& symbol) const {
  if (mappable_modules_->contains(symbol.module_file_name)) {
    if (const auto it = function_name_to_key_->find(symbol.function_name);
        it != function_name_to_key_->end()) {
      return it->second;
    }
  }
  return symbol.function_name;
}

const absl::flat_hash_map<std::string,
                          std::string>* D3D11DummyFunctionSymbolToKey::kDirectXToDxvkNames =
    new absl::flat_hash_map<std::string, std::string>{
        {"CContext::TID3D11DeviceContext_ClearRenderTargetView_<1>",
         "dxvk::D3D11DeviceContext::ClearRenderTargetView(ID3D11RenderTargetView*, float const*)"},
        {"CContext::TID3D11DeviceContext_SetShader_<1,0>",
         "dxvk::D3D11DeviceContext::PSSetShader(ID3D11PixelShader*, ID3D11ClassInstance* const*, "
         "unsigned int)"},
        {"CContext::TID3D11DeviceContext_Draw_<9>(ID3D11DeviceContext5 *,unsigned int,unsigned "
         "int)",
         "dxvk::D3D11DeviceContext::Draw(unsigned int, unsigned int)"},
        {"CContext::TID3D11DeviceContext_SetShader_<1,4>",
         "dxvk::D3D11DeviceContext::VSSetShader(ID3D11VertexShader*, ID3D11ClassInstance* const*, "
         "unsigned int)"},
        {"CDXGISwapChain::Present",
         "dxvk::D3D11SwapChain::Present(unsigned int, unsigned int, DXGI_PRESENT_PARAMETERS "
         "const*)"}};

const absl::flat_hash_set<std::string>* D3D11DummyFunctionSymbolToKey::kMappableModules =
    new absl::flat_hash_set<std::string>{"d3d11", "dxgi"};

}  // namespace orbit_mizar_data