//
//  mainwindow.cpp
//  ShadowClash
//
//  Created by TheWanderingCoel on 2018/6/12.
//  Copyright © 2019 Coel Wu. All rights reserved.
//

#include "apirequest.h"
#include "configmanager.h"
#include "mainwindow.h"
#include "proxyconfighelpermanager.h"
#include "systemtray.h"
#include "ui_mainwindow.h"

#include <QWebEngineProfile>

#if defined(Q_OS_MAC)
MainWindow::MainWindow(QWidget *parent)
    : CFramelessWindow(parent)
    , ui(new Ui::MainWindow)
{
#else
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
#endif

// Mac Os Only
#if defined(Q_OS_MACOS) || defined(Q_OS_MAC)
    // Hide Min Button
    this->setMinBtnEnabled(false);
    // Hide Max Button
    this->setZoomBtnEnabled(false);
    // Close Button Do not quit
    this->setCloseBtnQuit(false);
#endif

    // Prevent window being resized
    this->setFixedSize(920,580);

    m_pWebView = new QWebEngineView(this);
    // Set WebView Size
    m_pWebView->setGeometry(0,0,920,580);
    // Disable Right Click
    m_pWebView->setContextMenuPolicy(Qt::NoContextMenu);
    // Clear Cache
    m_pWebView->page()->profile()->clearHttpCache();
    // Finally show the view
    m_pWebView->load(QUrl(ConfigManager::apiUrl + "/ui"));

}

MainWindow::~MainWindow()
{
    delete ui;
}
