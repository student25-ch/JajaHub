#
#Copyright (C) 2026 杨子桐
#
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU Affero General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU Affero General Public License for more details.
#
#You should have received a copy of the GNU Affero General Public License
#along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
import random
import time
import smtplib
import jwt
from datetime import datetime, timedelta, timezone
from email.mime.text import MIMEText
from email.header import Header

from flask import Flask, request, jsonify

from config import (
    SMTP_HOST, SMTP_PORT, SMTP_USER, SMTP_PASSWORD, SMTP_FROM_NAME,
    ALIYUN_ACCESS_KEY_ID, ALIYUN_ACCESS_KEY_SECRET,
    ALIYUN_SIGN_NAME, ALIYUN_TEMPLATE_CODE,
    JWT_SECRET, JWT_ALGORITHM, JWT_EXPIRE_HOURS,
    code_store
)

app = Flask(__name__)

CODE_TTL = 300  # 验证码有效期 5 分钟


# ================= 验证码 =================
def generate_code():
    return "".join([str(random.randint(0, 9)) for _ in range(6)])


def store_code(key: str, code: str):
    code_store[key] = {"code": code, "expires": time.time() + CODE_TTL}


def verify_code(key: str, code: str) -> bool:
    entry = code_store.get(key)
    if not entry:
        return False
    if time.time() > entry["expires"]:
        del code_store[key]
        return False
    if entry["code"] != code:
        return False
    del code_store[key]
    return True


# ================= JWT =================
def create_jwt(user_id: str, login_type: str) -> str:
    payload = {
        "sub": user_id,
        "type": login_type,
        "iat": datetime.now(timezone.utc),
        "exp": datetime.now(timezone.utc) + timedelta(hours=JWT_EXPIRE_HOURS),
    }
    return jwt.encode(payload, JWT_SECRET, algorithm=JWT_ALGORITHM)


def decode_jwt(token: str):
    try:
        return jwt.decode(token, JWT_SECRET, algorithms=[JWT_ALGORITHM])
    except jwt.PyJWTError:
        return None


# ================= 发送 =================
def send_email_code(to_email: str, code: str) -> bool:
    try:
        msg = MIMEText(f"你的 JajaHub 验证码是：{code}，5 分钟内有效。", "plain", "utf-8")
        msg["Subject"] = Header("JajaHub 邮箱验证码", "utf-8")
        msg["From"] = f"{SMTP_FROM_NAME} <{SMTP_USER}>"
        msg["To"] = to_email

        with smtplib.SMTP_SSL(SMTP_HOST, SMTP_PORT) as server:
            server.login(SMTP_USER, SMTP_PASSWORD)
            server.sendmail(SMTP_USER, [to_email], msg.as_string())
        return True
    except Exception as e:
        print(f"[邮件发送失败] {e}")
        return False


def send_sms_code(phone: str, code: str) -> bool:
    if not ALIYUN_ACCESS_KEY_ID or not ALIYUN_ACCESS_KEY_SECRET:
        print("[短信] 未配置阿里云 AccessKey，跳过发送")
        return False
    try:
        from alibabacloud_dysmsapi20170525.client import Client
        from alibabacloud_tea_openapi import models as open_api_models
        from alibabacloud_dysmsapi20170525 import models as dysmsapi_models

        config = open_api_models.Config(
            access_key_id=ALIYUN_ACCESS_KEY_ID,
            access_key_secret=ALIYUN_ACCESS_KEY_SECRET
        )
        config.endpoint = "dysmsapi.aliyuncs.com"
        client = Client(config)

        req = dysmsapi_models.SendSmsRequest(
            phone_numbers=phone,
            sign_name=ALIYUN_SIGN_NAME,
            template_code=ALIYUN_TEMPLATE_CODE,
            template_param=f'{{"code":"{code}"}}'
        )
        resp = client.send_sms(req)
        if resp.body.code == "OK":
            return True
        print(f"[短信失败] {resp.body.code} - {resp.body.message}")
        return False
    except Exception as e:
        print(f"[短信异常] {e}")
        return False


# ================= 接口 =================
@app.route("/api/send-code", methods=["POST"])
def send_code():
    data = request.get_json()
    if not data:
        return jsonify({"error": "请求体为空"}), 400

    phone = data.get("phone", "").strip()
    email = data.get("email", "").strip()

    if phone:
        code = generate_code()
        store_code(f"phone:{phone}", code)
        ok = send_sms_code(phone, code)
        if not ok:
            print(f"[调试] 手机号 {phone} 的验证码：{code}")
        return jsonify({"status": "sent", "channel": "sms"})

    if email:
        code = generate_code()
        store_code(f"email:{email}", code)
        ok = send_email_code(email, code)
        if not ok:
            return jsonify({"error": "邮件发送失败，请检查 SMTP 配置"}), 500
        return jsonify({"status": "sent", "channel": "email"})

    return jsonify({"error": "需要提供 phone 或 email"}), 400


@app.route("/api/login/phone", methods=["POST"])
def login_phone():
    data = request.get_json() or {}
    phone = data.get("phone", "").strip()
    code = data.get("code", "").strip()

    if not phone or not code:
        return jsonify({"error": "手机号和验证码必填"}), 400

    if not verify_code(f"phone:{phone}", code):
        return jsonify({"error": "验证码错误或已过期"}), 401

    token = create_jwt(phone, "phone")
    return jsonify({"token": token})


@app.route("/api/login/email", methods=["POST"])
def login_email():
    data = request.get_json() or {}
    email = data.get("email", "").strip()
    code = data.get("code", "").strip()
    password = data.get("password", "").strip()

    if not email:
        return jsonify({"error": "邮箱必填"}), 400

    if code:
        if not verify_code(f"email:{email}", code):
            return jsonify({"error": "验证码错误或已过期"}), 401
        token = create_jwt(email, "email")
        return jsonify({"token": token})

    if password:
        return jsonify({"error": "密码登录暂未实现，请使用验证码"}), 501

    return jsonify({"error": "需要验证码或密码"}), 400


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=8080, debug=True)
