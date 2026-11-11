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
import os

# ---------- 邮件 SMTP ----------
SMTP_HOST = "smtp.qq.com"
SMTP_PORT = 465
SMTP_USER = "asd24718@qq.com"
SMTP_PASSWORD = "zdizwyhsiqtoeacj"   # 不是QQ密码
SMTP_FROM_NAME = "JajaHub"

# ---------- 阿里云短信（可选） ----------
ALIYUN_ACCESS_KEY_ID = os.getenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "")
ALIYUN_ACCESS_KEY_SECRET = os.getenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "")
ALIYUN_SIGN_NAME = "你的短信签名"
ALIYUN_TEMPLATE_CODE = "SMS_xxxxxx"

# ---------- JWT ----------
JWT_SECRET = "换成你自己的长随机字符串"
JWT_ALGORITHM = "HS256"
JWT_EXPIRE_HOURS = 24

# ---------- 验证码内存存储 ----------
code_store = {}
