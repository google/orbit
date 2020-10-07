// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const path = require("path");
const process = require("process");
const fs = require("fs");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const QrcGeneratorPlugin = require("./plugins/qrc-generator");
const MinimizeWritesPlugin = require("./plugins/minimize-writes");

if (
  !process.env.ORBIT_BUILD_FOLDER &&
  !fs.existsSync(path.resolve(__dirname, "qtwebchannel", "qwebchannel.js"))
) {
  console.error(`Could not find qtwebchannel/qwebchannel.js!
  
Webpack needs a version of "qwebchannel.js" which fits the currently used
version of Qt. Our build system automatically extracts the correct version into
WebUI/qtwebchannel/qwebchannel.js inside of the Orbit build directory.
  
You have two options:
  - Either set the ORBIT_BUILD_FOLDER environment variable and have it 
    point to your corresponding Orbit-build directory,
    (i.e. ${path.resolve(__dirname, "..", "build_default_relwithdebinfo")})
  - Or copy qtwebchannel/qwebchannel.js from your Orbit build directory into 
    the WebUI source directory (${__dirname}).
`);
  process.exit();
}

const qwebchannel_search_path = (() => {
  if (process.env.ORBIT_BUILD_FOLDER) {
    return path.resolve(process.env.ORBIT_BUILD_FOLDER, "WebUI");
  } else {
    return __dirname;
  }
})();

const config = {
  mode: "development",
  entry: {
    CodeView: "./src/CodeView",
  },
  output: {
    filename: "[name]/bundle.js",
    path: path.resolve(__dirname, "dist"),
  },
  resolve: {
    modules: ["node_modules", qwebchannel_search_path],
    extensions: [".ts", ".js", ".json", ".css"],
  },
  plugins: [
    new HtmlWebpackPlugin({
      chunks: ["CodeView"],
      filename: "CodeView/index.html",
      template: "src/CodeView.hbs",
      title: "CodeView",
    }),
    new QrcGeneratorPlugin(),
    new MinimizeWritesPlugin(),
  ],
  devServer: {
    contentBase: path.join(__dirname, "dist"),
    compress: true,
    port: 9000,
    openPage: "/CodeView/",
  },
  module: {
    rules: [
      {
        test: /\.[jt]s$/,
        exclude: /node_modules/,
        use: {
          loader: "babel-loader",
        },
      },
      {
        test: /\.css$/,
        use: ["style-loader", "css-loader"],
      },
      {
        test: /\.hbs$/,
        loader: "handlebars-loader",
      },
    ],
  },
};

module.exports = (env, args) => {
  const is_development = () => {
    return !args.mode || args.mode === "development";
  };

  if (!is_development()) {
    config.mode = "production";
    config.output.path = path.resolve(
      process.env.ORBIT_BUILD_FOLDER,
      "WebUI",
      "dist"
    );
    config.output.publicPath = "qrc:///WebUI/";
  }

  return config;
};
