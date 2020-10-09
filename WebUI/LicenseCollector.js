// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const license_checker = require("license-checker");
const fs = require("fs").promises;
const process = require("process");

const separator =
  "\n\n================================================================================";

const arguments = process.argv.slice(2);
let output_file = null;
let csv_output_file = null;

if (arguments.length == 0) {
  output_file = process.stdout;
} else if (arguments.length == 1) {
  output_file = fs.open(arguments[0], "w");
} else if (arguments.length == 2) {
  output_file = fs.open(arguments[0], "w");
  csv_output_file = fs.open(arguments[1], "w");
} else {
  throw "The number of command line arguments is invalid. Either provide an output file path or nothing!";
}

const write_file = async (file, content) => {
  const handle = await Promise.resolve(file);
  await Promise.resolve(handle.write(content));
  if (handle.close) {
    await Promise.resolve(handle.close());
  }
};

const write_notice_file = async (content) =>
  await write_file(output_file, content);
const write_license_table = async (content) =>
  await write_file(csv_output_file, content);

license_checker.init(
  {
    start: __dirname,
    production: true,
  },
  (err, packages) => {
    if (err) {
      console.log(err);
    } else {
      const relevant_package_names = Object.keys(packages)
          .filter((package_name) => !package_name.startsWith("orbit-"));

      Promise.all(
        relevant_package_names
          .map(async (package_name) => {
            const package = packages[package_name];

            const file = await fs.open(package.licenseFile, "r");
            const license_text = await file.readFile();
            const label = package_name.split("@", 1)[0];

            return `${separator}\nName: ${label}\nURL:${package.repository}\n\n${license_text}`;
          })
      ).then((parts) => {
        const content = parts.reduce((out, str) => out + str, "");
        write_notice_file(content);
      });

      if (csv_output_file) {
        const content = relevant_package_names.map((package_name) => {
            const package = packages[package_name];
            const label = package_name.split("@", 1)[0];

            return `${label},${package.repository},${package.licenses}`;
        }).reduce((out, line) => `${out}\n${line}`, "name,url,license");

        write_license_table(content);
      }
    }
  }
);
