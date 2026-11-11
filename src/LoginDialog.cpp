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
#include "LoginDialog.h"
#include "GitHubAuth.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();

    m_auth = new GitHubAuth(this);
    connect(m_auth, &GitHubAuth::authenticated, this, &LoginDialog::onGitHubAuthSuccess);
    connect(m_auth, &GitHubAuth::failed, this, &LoginDialog::onGitHubAuthFailed);
    connect(m_auth, &GitHubAuth::userCodeReady, this,
            [this](const QString &uri, const QString &code) {
                m_githubButton->setText("等待授权中...");
                m_deviceCodeLabel->setText(
                    QString("请在浏览器中输入代码：<b>%1</b><br/>地址：%2").arg(code, uri));
            });
}

void LoginDialog::setupUi()
{
    setWindowTitle("登录 JajaHub");
    resize(460, 420);

    auto *layout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);

    // ---------- GitHub 页 ----------
    auto *githubPage = new QWidget(this);
    auto *githubLayout = new QVBoxLayout(githubPage);
    githubLayout->addWidget(new QLabel("使用 GitHub 设备流授权登录", githubPage));
    m_githubButton = new QPushButton("开始 GitHub 授权", githubPage);
    githubLayout->addWidget(m_githubButton);
    m_deviceCodeLabel = new QLabel(githubPage);
    m_deviceCodeLabel->setTextFormat(Qt::RichText);
    m_deviceCodeLabel->setWordWrap(true);
    githubLayout->addWidget(m_deviceCodeLabel);
    githubLayout->addStretch();
    connect(m_githubButton, &QPushButton::clicked, this, &LoginDialog::onGitHubLoginClicked);
    m_tabs->addTab(githubPage, "GitHub");

    // ---------- 手机页 ----------
    auto *phonePage = new QWidget(this);
    auto *phoneLayout = new QFormLayout(phonePage);
    m_phoneEdit = new QLineEdit(phonePage);
    m_phoneEdit->setPlaceholderText("手机号");
    m_codeEdit = new QLineEdit(phonePage);
    m_codeEdit->setPlaceholderText("验证码");

    auto *phoneCodeRow = new QHBoxLayout();
    m_sendPhoneCodeButton = new QPushButton("获取验证码", phonePage);
    phoneCodeRow->addWidget(m_codeEdit);
    phoneCodeRow->addWidget(m_sendPhoneCodeButton);

    m_phoneLoginButton = new QPushButton("登录 / 注册", phonePage);
    phoneLayout->addRow("手机号", m_phoneEdit);
    phoneLayout->addRow("验证码", phoneCodeRow);
    phoneLayout->addRow(m_phoneLoginButton);

    connect(m_sendPhoneCodeButton, &QPushButton::clicked, this, &LoginDialog::onSendPhoneCodeClicked);
    connect(m_phoneLoginButton, &QPushButton::clicked, this, &LoginDialog::onPhoneLoginClicked);
    m_tabs->addTab(phonePage, "手机验证码");

    // ---------- 邮箱页 ----------
    auto *emailPage = new QWidget(this);
    auto *emailLayout = new QFormLayout(emailPage);
    m_emailEdit = new QLineEdit(emailPage);
    m_emailEdit->setPlaceholderText("邮箱");
    m_emailCodeEdit = new QLineEdit(emailPage);
    m_emailCodeEdit->setPlaceholderText("验证码");

    auto *emailCodeRow = new QHBoxLayout();
    m_sendEmailCodeButton = new QPushButton("获取验证码", emailPage);
    emailCodeRow->addWidget(m_emailCodeEdit);
    emailCodeRow->addWidget(m_sendEmailCodeButton);

    m_emailLoginButton = new QPushButton("登录 / 注册", emailPage);
    emailLayout->addRow("邮箱", m_emailEdit);
    emailLayout->addRow("验证码", emailCodeRow);
    emailLayout->addRow(m_emailLoginButton);

    connect(m_sendEmailCodeButton, &QPushButton::clicked, this, &LoginDialog::onSendEmailCodeClicked);
    connect(m_emailLoginButton, &QPushButton::clicked, this, &LoginDialog::onEmailLoginClicked);
    m_tabs->addTab(emailPage, "邮箱验证码");

    layout->addWidget(m_tabs);
}

void LoginDialog::postJson(const QString &endpoint, const QJsonObject &payload,
                           std::function<void(const QJsonObject &)> onSuccess,
                           std::function<void(const QString &)> onError)
{
    auto *nam = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl(m_backendBaseUrl + endpoint));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto *reply = nam->post(req, QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();
        nam->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            onError(reply->errorString());
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        onSuccess(doc.object());
    });
}

// ---------- GitHub ----------
void LoginDialog::onGitHubLoginClicked()
{
    m_githubButton->setEnabled(false);
    m_githubButton->setText("正在请求验证码...");
    m_deviceCodeLabel->clear();
    m_auth->startAuthorization();
}

void LoginDialog::onGitHubAuthSuccess(const QString &token)
{
    m_accessToken = token;
    m_loginType = "github";
    accept();
}

void LoginDialog::onGitHubAuthFailed(const QString &error)
{
    m_githubButton->setEnabled(true);
    m_githubButton->setText("开始 GitHub 授权");
    QMessageBox::warning(this, "GitHub 登录失败", error);
}

// ---------- 手机 ----------
void LoginDialog::onSendPhoneCodeClicked()
{
    const QString phone = m_phoneEdit->text().trimmed();
    if (phone.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先填写手机号");
        return;
    }
    QJsonObject payload;
    payload["phone"] = phone;

    m_sendPhoneCodeButton->setEnabled(false);
    m_sendPhoneCodeButton->setText("发送中...");

    postJson("/api/send-code", payload,
        [this](const QJsonObject &) {
            m_sendPhoneCodeButton->setEnabled(true);
            m_sendPhoneCodeButton->setText("获取验证码");
            QMessageBox::information(this, "已发送", "验证码已发送（未配置短信时请查看后端控制台）");
        },
        [this](const QString &err) {
            m_sendPhoneCodeButton->setEnabled(true);
            m_sendPhoneCodeButton->setText("获取验证码");
            QMessageBox::warning(this, "发送失败", err);
        });
}

void LoginDialog::onPhoneLoginClicked()
{
    const QString phone = m_phoneEdit->text().trimmed();
    const QString code = m_codeEdit->text().trimmed();
    if (phone.isEmpty() || code.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写手机号和验证码");
        return;
    }
    QJsonObject payload;
    payload["phone"] = phone;
    payload["code"] = code;

    m_phoneLoginButton->setEnabled(false);
    m_phoneLoginButton->setText("登录中...");

    postJson("/api/login/phone", payload,
        [this](const QJsonObject &resp) {
            m_phoneLoginButton->setEnabled(true);
            m_phoneLoginButton->setText("登录 / 注册");
            if (resp.contains("token")) {
                m_accessToken = resp["token"].toString();
                m_loginType = "phone";
                accept();
            } else {
                QMessageBox::warning(this, "登录失败",
                    resp["error"].toString("后端未返回 token"));
            }
        },
        [this](const QString &err) {
            m_phoneLoginButton->setEnabled(true);
            m_phoneLoginButton->setText("登录 / 注册");
            QMessageBox::warning(this, "网络错误", "无法连接后端服务：\n" + err);
        });
}

// ---------- 邮箱 ----------
void LoginDialog::onSendEmailCodeClicked()
{
    const QString email = m_emailEdit->text().trimmed();
    if (email.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先填写邮箱");
        return;
    }
    QJsonObject payload;
    payload["email"] = email;

    m_sendEmailCodeButton->setEnabled(false);
    m_sendEmailCodeButton->setText("发送中...");

    postJson("/api/send-code", payload,
        [this](const QJsonObject &) {
            m_sendEmailCodeButton->setEnabled(true);
            m_sendEmailCodeButton->setText("获取验证码");
            QMessageBox::information(this, "已发送", "验证码已发送到邮箱，请查收");
        },
        [this](const QString &err) {
            m_sendEmailCodeButton->setEnabled(true);
            m_sendEmailCodeButton->setText("获取验证码");
            QMessageBox::warning(this, "发送失败", err);
        });
}

void LoginDialog::onEmailLoginClicked()
{
    const QString email = m_emailEdit->text().trimmed();
    const QString code = m_emailCodeEdit->text().trimmed();
    if (email.isEmpty() || code.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写邮箱和验证码");
        return;
    }
    QJsonObject payload;
    payload["email"] = email;
    payload["code"] = code;

    m_emailLoginButton->setEnabled(false);
    m_emailLoginButton->setText("登录中...");

    postJson("/api/login/email", payload,
        [this](const QJsonObject &resp) {
            m_emailLoginButton->setEnabled(true);
            m_emailLoginButton->setText("登录 / 注册");
            if (resp.contains("token")) {
                m_accessToken = resp["token"].toString();
                m_loginType = "email";
                accept();
            } else {
                QMessageBox::warning(this, "登录失败",
                    resp["error"].toString("后端未返回 token"));
            }
        },
        [this](const QString &err) {
            m_emailLoginButton->setEnabled(true);
            m_emailLoginButton->setText("登录 / 注册");
            QMessageBox::warning(this, "网络错误", "无法连接后端服务：\n" + err);
        });
}