#!/bin/env python3

import hashlib

passwords = [
    "123456",
    "admin",
    "12345678",
    "123456789",
    "12345",
    "password",
    "Aa123456",
    "1234567890",
    "Pass@123",
    "admin123",
    "1234567",
    "123123",
    "111111",
    "12345678910",
    "P@ssw0rd",
    "Password",
    "Aa@123456",
    "admintelecom",
    "Admin@123",
    "112233",
]

known_hash = "f91e15dbec69fc40f81f0876e7009648"

for password in passwords:
    password_hash = hashlib.md5(password.encode()).hexdigest()

    if password_hash == known_hash:
        print(password)
