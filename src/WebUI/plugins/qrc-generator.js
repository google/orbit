// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


/*
  The QrcGeneratorPlugin for Webpack generates a Qt resource file
  listing all the assets produced by Webpack.

  This allows embedding of a webpack-generated bundle into a Qt-based
  C++-application.

  Qt's WebEngine module supports loading websites from QRC resources
  out-of-the-box. So this is a convenient way to embed a static webpage.
*/
class QrcGeneratorPlugin {
    apply(compiler) {
        const onEmit = (comp) => {
            const prefix = '<!DOCTYPE RCC>\n<RCC version="1.0">\n  <qresource prefix="/WebUI">\n';
            const suffix = '  </qresource>\n</RCC>\n';
            const files = Object.keys(comp.assets).reduce((str, filename) => str + `    <file>${filename}</file>\n`, '');
            const contents = prefix + files + suffix;

            comp.assets['WebUI.qrc'] = {
                source: () => {
                    return contents;
                },
                size: () => {
                    return contents.length;
                }
            };
        };

        compiler.hooks.emit.tap('qrc-generator-plugin', onEmit);
    }
}

module.exports = QrcGeneratorPlugin;