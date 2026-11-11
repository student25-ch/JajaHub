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
#include "GitHubAuth.h"

#include <QOAuth2DeviceAuthorizationFlow>
#include <QAbstractOAuth>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkRequest>

// 替换为你在 GitHub OAuth App 中拿到的 Client ID（设备流不需要 Client Secret）
static const QString CLIENT_ID = "Ov23likEyUgtb0wPWP5x";

GitHubAuth::GitHubAuth(QObject *parent)
    : QObject(parent)
{
    m_flow = new QOAuth2DeviceAuthorizationFlow(this);

    m_flow->setAuthorizationUrl(QUrl("https://github.com/login/device/code"));
    m_flow->setTokenUrl(QUrl("https://github.com/login/oauth/access_token"));
    m_flow->setClientIdentifier(CLIENT_ID);

    m_flow->setNetworkRequestModifier(this, [](QNetworkRequest &req, QAbstractOAuth::Stage) {
        req.setRawHeader("Accept", "application/json");
    });

    connect(m_flow, &QOAuth2DeviceAuthorizationFlow::authorizeWithUserCode,
            this, [this](const QUrl &verificationUrl, const QString &userCode, const QUrl &) {
                emit userCodeReady(verificationUrl.toString(), userCode);
                QDesktopServices::openUrl(verificationUrl);
                m_flow->startTokenPolling();
            });

    connect(m_flow, &QOAuth2DeviceAuthorizationFlow::granted, this, [this]() {
        emit authenticated(m_flow->token());
    });

    connect(m_flow, &QOAuth2DeviceAuthorizationFlow::error, this,
            [this](const QString &error, const QString &desc, const QUrl &) {
                emit failed(error + ": " + desc);
            });
}

void GitHubAuth::startAuthorization()
{
    m_flow->grant();
}