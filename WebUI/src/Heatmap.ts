// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import "./Heatmap.css";
import Prism from "prismjs";

export default function (view: any) {
  Prism.hooks.add("before-sanity-check", (env) => {
    const pre = env.element.parentNode;

    if (!/pre/i.test(pre?.nodeName)) {
      return;
    }

    const heatmap = pre.querySelector(".heatmap-wrapper");
    if (heatmap) {
      heatmap.remove();
    }

    const highlights = pre.querySelector(".line-heatmap-highlight-wrapper");
    if (highlights) {
      highlights.remove();
    }
  });

  Prism.hooks.add("complete", (env) => {
    const pre = <HTMLElement>env.element.parentNode;

    if (
      !/pre/i.test(pre?.nodeName) ||
      !view ||
      !pre?.classList.contains("hit-count")
    ) {
      return;
    }

    const heatmap_wrapper = document.createElement("div");
    heatmap_wrapper.className = "heatmap-wrapper";

    interface FormatFunction {
      (value: number): string;
    }

    const AddColumn = (
      values: number[],
      title: string,
      format: FormatFunction,
      id: string
    ) => {
      const wrapper = document.createElement("div");
      wrapper.className = "heatmap-row";
      wrapper.id = id;

      const title_el = document.createElement("span");
      title_el.textContent = title;
      title_el.className = "title";
      wrapper.appendChild(title_el);

      values
        .map((value: number) => {
          const span = document.createElement("span");
          span.textContent = format(value);
          return span;
        })
        .forEach((child: HTMLSpanElement) => wrapper.appendChild(child));

      heatmap_wrapper.appendChild(wrapper);
    };

    if (view.hit_counts) {
      AddColumn(
        view.hit_counts,
        "Samples",
        (value: number) => `${value}`,
        "hit_counts"
      );
    }

    if (view.hit_ratios) {
      AddColumn(
        view.hit_ratios,
        "Function",
        (value: number) => `${(value * 100).toFixed(2)} %`,
        "hit_ratios"
      );
    }

    if (view.total_hit_ratios) {
      AddColumn(
        view.total_hit_ratios,
        "Total",
        (value: number) => `${(value * 100).toFixed(2)} %`,
        "total_hit_ratios"
      );
    }

    pre.appendChild(heatmap_wrapper);

    const line_highlight_wrapper = document.createElement("div");
    line_highlight_wrapper.className = "line-heatmap-highlight-wrapper";

    const hit_ratio_wrapper = pre.querySelector("#hit_ratios");

    view.hit_ratios
      .map((hit_ratio: number, idx: number) => {
        if (hit_ratio == 0) {
          return null;
        }

        const hit_ratio_element = <HTMLElement>(
          hit_ratio_wrapper.children[idx + 1]
        );

        const highlight_line = document.createElement("div");
        highlight_line.classList.add("line-heatmap-highlight");
        highlight_line.style.top = `${hit_ratio_element.offsetTop}px`;
        highlight_line.style.height = `${hit_ratio_element.offsetHeight}px`;

        const hit_ratio_sqrt = Math.sqrt(hit_ratio);
        highlight_line.style.background = `linear-gradient(to left, rgba(var(--accent-color), ${hit_ratio_sqrt}) 0%, rgba(var(--accent-color), ${
          hit_ratio_sqrt / 2
        }) 12%, rgba(var(--accent-color), 0) 90%)`;

        return highlight_line;
      })
      .filter((el: HTMLElement | null) => el !== null)
      .forEach((el: HTMLElement) => line_highlight_wrapper.appendChild(el));

    pre.querySelector("div.heatmap-wrapper").prepend(line_highlight_wrapper);
  });
}
