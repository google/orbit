// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import "./CodeView.css";
import "./Orbit.css";
import Prism from "prismjs";
import QWebChannel from "qtwebchannel/qwebchannel.js";
import x86asm from "./x86asm";
import Heatmap from "./Heatmap";

Prism.languages.x86asm = x86asm;

const CreateWebChannel = (transport) => {
  return new QWebChannel.QWebChannel(transport, (channel) => {
    const view = channel.objects.view;
    Heatmap(view);

    const update_view = () => {
      const {
        title,
        source_code,
        language,
        line_numbers_enabled,
        heatmap_enabled,
      } = view;
      document.querySelector("header").innerText = title;
      document
        .querySelector("main pre")
        .classList.toggle("line-numbers", line_numbers_enabled);
      document
        .querySelector("main pre")
        .classList.toggle("hit-count", heatmap_enabled);

      const source_el = document.getElementById("source");
      source_el.textContent = source_code;
      source_el.className = `language-${language}`;
      Prism.highlightElement(source_el, false);
      document.querySelector("main").scrollTop = 0;
    };

    update_view();
    view.sourceCodeChanged.connect(update_view);
  });
};

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("close").onclick = () => {
    window.close();
  };

  if (typeof qt !== "undefined" && qt.webChannelTransport) {
    // qt.webChannelTransport is only available when loaded in QWebEngine.
    // So we fall back to websocket in a generic browser for development purposes.
    CreateWebChannel(qt.webChannelTransport);
  } else {
    const websocket_port = 44738;
    const socket = new WebSocket(`ws://localhost:${websocket_port}`);
    socket.onopen = () => CreateWebChannel(socket);
  }
});
