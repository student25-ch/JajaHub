# JajaHub

一个基于 Qt6 / C++ 的桌面端 GitHub 客户端。

作者：杨子桐

## 功能

- GitHub 设备流 OAuth 登录（无需 Client Secret）
- 个人主页展示该用户的 GitHub 仓库列表
- 手机验证码登录 / 注册（对接 Python 后端）
- 邮箱验证码登录 / 注册（对接 Python 后端）
- 后端签发真实 JWT

## 许可证

本项目使用 **AGPLv3** 许可证，详见 [LICENSE](LICENSE)。

## 环境要求

- CMake >= 3.16
- Qt 6.9 或更高版本（Widgets、Network、NetworkAuth）
- C++17 兼容编译器
- 配套后端：`JajaHub-Backend`（Python Flask）

## 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
运行

先启动后端（python app.py），再运行 ./JajaHub。

配置
src/GitHubAuth.cpp 中填入 GitHub OAuth App 的 Client ID（需开启 Device Flow）

src/LoginDialog.h 中 m_backendBaseUrl 指向后端地址，默认 http://127.0.0.1:8080