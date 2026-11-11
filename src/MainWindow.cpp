/*
 * Copyright (C) 2026 杨子桐
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "MainWindow.h"
#include "LoginDialog.h"
#include "GitHubApi.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
}

void MainWindow::setupUi()
{
    setWindowTitle("JajaHub — 杨子桐");
    resize(800, 600);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    auto *topBar = new QHBoxLayout();
    m_userLabel = new QLabel("未登录", central);
    m_loginButton = new QPushButton("登录", central);
    topBar->addWidget(m_userLabel);
    topBar->addStretch();
    topBar->addWidget(m_loginButton);
    layout->addLayout(topBar);

    m_repoList = new QListWidget(central);
    layout->addWidget(m_repoList);

    setCentralWidget(central);

    connect(m_loginButton, &QPushButton::clicked, this, &MainWindow::onLoginClicked);

    m_api = new GitHubApi(this);
    connect(m_api, &GitHubApi::reposFetched, this, &MainWindow::onReposFetched);
    connect(m_api, &GitHubApi::fetchFailed, this, &MainWindow::onFetchFailed);
}

void MainWindow::onLoginClicked()
{
    LoginDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_accessToken = dialog.accessToken();
        m_userLabel->setText("已登录");
        m_repoList->clear();
        if (dialog.loginType() == "github" && !m_accessToken.isEmpty()) {
            m_api->fetchUserRepos(m_accessToken);
        } else {
            m_repoList->addItem("已通过 " + dialog.loginType() + " 登录（JWT 已保存）");
        }
    }
}

void MainWindow::onReposFetched(const QJsonArray &repos)
{
    clearRepoList();
    for (const auto &item : repos) {
        const QJsonObject repo = item.toObject();
        const QString name = repo.value("name").toString();
        const QString desc = repo.value("description").toString();
        const int stars = repo.value("stargazers_count").toInt();
        const QString lang = repo.value("language").toString();

        QString line = QString("%1  ★%2  [%3]").arg(name).arg(stars).arg(lang);
        if (!desc.isEmpty())
            line += "  —  " + desc;

        m_repoList->addItem(line);
    }
    if (repos.isEmpty())
        m_repoList->addItem("（该用户暂无公开仓库）");
}

void MainWindow::onFetchFailed(const QString &error)
{
    QMessageBox::warning(this, "获取仓库失败", error);
}

void MainWindow::clearRepoList()
{
    m_repoList->clear();
}