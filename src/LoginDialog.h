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
#pragma once

#include <QDialog>
#include <QString>
#include <QJsonObject>
#include <functional>

class QTabWidget;
class QLineEdit;
class QPushButton;
class QLabel;
class GitHubAuth;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString accessToken() const { return m_accessToken; }
    QString loginType() const { return m_loginType; }

private slots:
    void onGitHubLoginClicked();
    void onGitHubAuthSuccess(const QString &token);
    void onGitHubAuthFailed(const QString &error);
    void onSendPhoneCodeClicked();
    void onSendEmailCodeClicked();
    void onPhoneLoginClicked();
    void onEmailLoginClicked();

private:
    void setupUi();
    void postJson(const QString &endpoint, const QJsonObject &payload,
                  std::function<void(const QJsonObject &)> onSuccess,
                  std::function<void(const QString &)> onError);

    QTabWidget *m_tabs = nullptr;

    // GitHub
    QPushButton *m_githubButton = nullptr;
    QLabel *m_deviceCodeLabel = nullptr;

    // 手机
    QLineEdit *m_phoneEdit = nullptr;
    QLineEdit *m_codeEdit = nullptr;
    QPushButton *m_sendPhoneCodeButton = nullptr;
    QPushButton *m_phoneLoginButton = nullptr;

    // 邮箱
    QLineEdit *m_emailEdit = nullptr;
    QLineEdit *m_emailCodeEdit = nullptr;
    QPushButton *m_sendEmailCodeButton = nullptr;
    QPushButton *m_emailLoginButton = nullptr;

    GitHubAuth *m_auth = nullptr;
    QString m_accessToken;
    QString m_loginType;

    QString m_backendBaseUrl = "http://127.0.0.1:8080";
};