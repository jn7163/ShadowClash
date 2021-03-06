//
//  enhancemodemanager.cpp
//  ShadowClash
//
//  Created by TheWanderingCoel on 2019/12/4.
//  Copyright © 2019 Coel Wu. All rights reserved.
//

#include "appversionutil.h"
#include "clashconfig.h"
#include "configmanager.h"
#include "enhancemodemanager.h"
#include "logger.h"
#include "paths.h"
#include "yaml-cpp/yaml.h"

#include <iostream>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QHostInfo>

using namespace std;

QString EnhanceModeManager::gateway;

void EnhanceModeManager::install()
{
    QString dir = Paths::configFolderPath;
    QString temp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

#ifdef Q_OS_WIN
    QProcess *task;

    if (!QFile::exists(dir + "tun2socks.exe")) {
        QFile::copy(":/tun2socks.exe",dir + "tun2socks.exe");
    }

    if (AppVersionUtil::isFirstLaunch) {
        // Install Tap Driver
        QFile::copy(":/tap-windows-9.22.1-I602.exe", temp+ "tap-windows-9.22.1-I602.exe");
        task->start(temp + "tap-windows-9.22.1-I602.exe");
    }

#elif defined(Q_OS_MAC)
    if (!QFile::exists(dir + "tun2socks")) {
        QFile::copy(":/tun2socks",dir + "tun2socks");
    }

    if (!QFile::exists(dir + "tun2socksHelper")) {
        QFile::copy(":/tun2socksHelper",dir + "tun2socksHelper");
    }

#elif defined(Q_OS_LINUX)
    if (!QFile::exists(dir + "tun2socks")) {
        QFile::copy(":/tun2socksLinux",dir + "tun2socks");
    }
#endif
}

void EnhanceModeManager::getDefaultGateway()
{
    QProcess *task = new QProcess;
    QStringList param;
#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
    param << "-c" << "route get default | grep gateway | awk '{print $2}'";
    task->start("bash", param);
    task->waitForFinished();
    EnhanceModeManager::gateway = task->readAllStandardOutput();
    EnhanceModeManager::gateway = EnhanceModeManager::gateway.remove("\n");
#endif

}

void EnhanceModeManager::startTun2socks()
{
    getDefaultGateway();

    QProcess *task = new QProcess;
    QStringList param;
    QRegExp pattern("^([a-zA-Z0-9-]+.)+([a-zA-Z])+$");

    QString path = Paths::configFolderPath + ConfigManager::selectConfigName + ".yaml";
    YAML::Node config = YAML::LoadFile(path.toStdString());
    const YAML::Node& proxies = config["Proxy"];
    QFile file(Paths::configFolderPath + "ip.txt");
    file.open(QIODevice::ReadWrite|QIODevice::Truncate);
    for (std::size_t i = 0; i < proxies.size(); i++) {
        const YAML::Node& proxy = proxies[i];
        // convert domain to ip address
        if (pattern.exactMatch(QString::fromStdString(proxy["server"].as<string>()))) {
            QString ip = QHostInfo::fromName(QString::fromStdString(proxy["server"].as<string>())).addresses().first().toString();
            file.write(ip.toUtf8());
        } else
        // ip address only
        {
           file.write(QString::fromStdString(proxy["server"].as<string>()).toUtf8());
        }
        file.write("\n");
    }
    file.close();

#if defined(Q_OS_WIN)
    Logger::log("startTun2socks", "debug");
    param << "-tunName" <<  "tun1" << "-tunAddr" << "240.0.0.2" << "-tunGw" << "240.0.0.1" << "-proxyType" << "socks" << "-proxyServer" << "127.0.0.1" + QString::number(ClashConfig::socketPort);
    task->startDetached(Paths::configFolderPath + "tun2socks.exe", param);
#elif defined(Q_OS_MAC)
    Logger::log("startTun2socks", "debug");
    param << "-tunName" <<  "utun1" << "-tunAddr" << "240.0.0.2" << "-tunGw" << "240.0.0.1" << "-proxyType" << "socks" << "-proxyServer" << "127.0.0.1:" + QString::number(ClashConfig::socketPort);
    task->startDetached(Paths::configFolderPath + "tun2socks", param);
    param.clear();
    param << Paths::configFolderPath << EnhanceModeManager::gateway << "start";
    task->start(Paths::configFolderPath + "tun2socksHelper", param);
    task->waitForFinished();
#elif defined(Q_OS_LINUX)
    param << "-tunName" <<  "tun1" << "-tunAddr" << "240.0.0.2" << "-tunGw" << "240.0.0.1" << "-proxyType" << "socks" << "-proxyServer" << "127.0.0.1:" + QString::number(ClashConfig::socketPort);
    task->startDetached(Paths::configFolderPath + "tun2socks", param);
    param << "bash" << Paths::configFolderPath + "tun2socks.sh" << Paths::configFolderPath << EnhanceModeManager::gateway << "start";
    task->start("kdesu", param);
    task->waitForFinished();
#endif
    QFile::remove(Paths::configFolderPath + "ip.txt");

}

void EnhanceModeManager::stopTun2socks()
{
    QProcess *task = new QProcess;
    QStringList param;

    QString path = Paths::configFolderPath + ConfigManager::selectConfigName + ".yaml";
    YAML::Node config = YAML::LoadFile(path.toStdString());
    const YAML::Node& proxies = config["Proxy"];
    QFile file(Paths::configFolderPath + "ip.txt");
    file.open(QIODevice::ReadWrite|QIODevice::Truncate);
    for (std::size_t i = 0; i < proxies.size(); i++) {
        const YAML::Node& proxy = proxies[i];
        file.write(QString::fromStdString(proxy["server"].as<string>()).toUtf8());
        file.write("\n");
    }
    file.close();
#if defined (Q_OS_WIN)
#elif defined(Q_OS_MAC)
    Logger::log("stopTun2socks", "debug");
    param.clear();
    param << Paths::configFolderPath << EnhanceModeManager::gateway << "stop";
    task->start(Paths::configFolderPath + "tun2socksHelper", param);
    task->waitForFinished();
#elif defined(Q_OS_LINUX)
    param << "bash" << Paths::configFolderPath + "tun2socks.sh" << Paths::configFolderPath << EnhanceModeManager::gateway << "stop";
    task->start("kdesu", param);
    task->waitForFinished();
#endif
    QFile::remove(Paths::configFolderPath + "ip.txt");
}

