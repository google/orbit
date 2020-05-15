// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QCoreApplication>
#include <QEventLoop>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <queue>
#include <thread>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/GgpClient.h"
#include "OrbitGgp/GgpInstance.h"
#include "OrbitGgp/GgpSshInfo.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSsh/ResultType.h"
#include "OrbitSsh/SshManager.h"

std::optional<GgpSshInfo> GetSshInfoSync(int index) {
  GgpClient::ResultOrQString<GgpClient> create_result = GgpClient::Create();
  if (!create_result) {
    ERROR("Unable to use ggp command line, error: %s",
          create_result.error().toStdString().c_str());
    return std::nullopt;
  }
  GgpClient client = std::move(create_result.value());
  LOG("Created ggp client");

  QEventLoop loop{};
  std::optional<GgpSshInfo> opt_ssh_info = std::nullopt;
  client.GetInstancesAsync(
      [&loop, &client, &opt_ssh_info,
       &index](GgpClient::ResultOrQString<QVector<GgpInstance>> vector) {
        if (!vector) {
          ERROR("%s", vector.error().toStdString().c_str());
          loop.quit();
          return;
        }

        if (vector.value().empty()) {
          ERROR("no reserved instances");
          loop.quit();
          return;
        }

        LOG("Got ggp instances");
        const GgpInstance& instance{vector.value()[index]};

        client.GetSshInformationAsync(
            instance, [&loop, &opt_ssh_info](
                          GgpClient::ResultOrQString<GgpSshInfo> result) {
              if (!result) {
                ERROR("%s", result.error().toStdString().c_str());
                loop.quit();
                return;
              }

              LOG("Got ggp ssh init data");
              opt_ssh_info = result.value();
              loop.quit();
            });
      });

  loop.exec();

  if (!opt_ssh_info) {
    ERROR("Could not get ssh info of instance");
    return std::nullopt;
  }

  return std::move(opt_ssh_info.value());
}

int main(int argc, char* argv[]) {
  QCoreApplication a{argc, argv};

  auto ssh_opt = GetSshInfoSync(0);
  if (!ssh_opt) {
    ERROR("Unable to get ggp ssh info");
    return -1;
  }
  GgpSshInfo ssh_info = std::move(*ssh_opt);

  OrbitSsh::Credentials credentials = {};

  credentials.host = ssh_info.host.toStdString();
  credentials.port = ssh_info.port;
  credentials.user = ssh_info.user.toStdString();
  credentials.known_hosts_path = ssh_info.known_hosts_path.toStdString();
  credentials.key_path = ssh_info.key_path.toStdString();

  std::queue<OrbitSsh::SshManager::Task> pre_tasks;
  pre_tasks.emplace(OrbitSsh::SshManager::Task{
      "ls -al /mnt/developer",
      [](std::string output) { LOG("Pre task output: %s", output.c_str()); },
      [](int exit_code) {
        LOG("Pre task finished with exit code %d", exit_code);
      }});

  OrbitSsh::SshManager::Task main_task = {
      "ls -al /mnt/developer/",
      [](std::string output) { LOG("Main task output: %s", output.c_str()); },
      [](int exit_code) { LOG("Man task exit code: %d", exit_code); }};

  OrbitSsh::SshManager ssh_handler(credentials, pre_tasks, main_task,
                                   {44766, 44755});

  for (int i = 0; i < 5000; i++) {
    ssh_handler.Tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  OrbitSsh::ResultType close_result;
  do {
    close_result = ssh_handler.Close();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  } while (close_result == OrbitSsh::ResultType::kAgain);

  return 0;
}