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

#include <QObject>
#include <QString>

class QOAuth2DeviceAuthorizationFlow;

class GitHubAuth : public QObject
{
    Q_OBJECT
public:
    explicit GitHubAuth(QObject *parent = nullptr);
    void startAuthorization();

signals:
    void userCodeReady(const QString &verificationUri, const QString &userCode);
    void authenticated(const QString &accessToken);
    void failed(const QString &error);

private:
    QOAuth2DeviceAuthorizationFlow *m_flow = nullptr;
};