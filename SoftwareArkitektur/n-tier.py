#!/usr/bin/env python3

class UserRepository:
    def __init__(self):
        # Simulated database
        self._users = {}

    def save_user(self, username, age):
        self._users[username] = age

    def get_user(self, username):
        return self._users.get(username)

class UserService:
    def __init__(self, user_repository):
        self.user_repository = user_repository

    def register_user(self, username, age):
        if age < 18:
            raise ValueError("User must be at least 18 years old")

        self.user_repository.save_user(username, age)

    def get_user_info(self, username):
        age = self.user_repository.get_user(username)
        if age is None:
            return "User not found"
        return f"User {username} is {age} years old"

repo = UserRepository()
service = UserService(repo)

try:
    service.register_user("alice", 25)
    service.register_user("bob", 15)  # Will fail
except ValueError as e:
    print("Error:", e)

print(service.get_user_info("alice"))
print(service.get_user_info("bob"))
