#!/usr/bin/env python3

from abc import ABC, abstractmethod

class Logger(ABC):
    @abstractmethod
    def log(self, message: str) -> None:
        pass

class FileLogger(Logger):
    def log(self, message: str) -> None:
        print(f"[FILE] {message}")

class ConsoleLogger(Logger):
    def log(self, message: str) -> None:
        print(f"[CONSOLE] {message}")

class OrderService:
    def __init__(self, logger: Logger):
        # Depends on abstraction, not concrete class
        self.logger = logger

    def create_order(self) -> None:
        self.logger.log("Order created")


# Swap implementations without changing OrderService
logger = ConsoleLogger()
service = OrderService(logger)
service.create_order()

logger = FileLogger()
service = OrderService(logger)
service.create_order()
