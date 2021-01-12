// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const fs = require('fs').promises;
const path = require('path');

/*
  MinimizeWritesPlugin is a webpack plugin for reducing unnecessary writes
  when emitting code and other assets.

  The plugin checks if the target file already exists and if it has the same
  contents that is supposed to be written and avoids the write in that case.

  The advantage: Filesystem modification times only change when the file's
  contents actually change - not on every invocation of webpack.
*/
class MinimizeWritesPlugin {
    apply(compiler) {
        const onEmit = async compilation => {

            const assets = compilation.assets;
            await Promise.all(Object.keys(assets).map(async filename => {
                const new_content = Buffer.from(assets[filename].source());

                try {
                    const destination_file_path = path.join(compilation.outputOptions.path, filename);
                    const old_content = await fs.readFile(destination_file_path);

                    if (old_content.compare(new_content) == 0) {
                        // If both file contents are the same, we don't need to write that file
                        delete assets[filename];
                    }
                } catch (_) {
                    // If the file doesn't exist or we can't read it, webpack should create/update it
                }
            }));
        };

        compiler.hooks.emit.tapPromise('minimize-writes-plugin', onEmit);
    }
}

module.exports = MinimizeWritesPlugin;