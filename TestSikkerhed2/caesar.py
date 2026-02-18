#!/bin/env python3

alphabet = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
            "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Æ", "Ø", "Å"]

message = "KRCKN OØ NOA WSXNÅA AKVAO ÅZØYQ S COØNOX"

for m_char in message:
    for index, a_char in enumerate(alphabet):
        if a_char == m_char:
            print(alphabet[(index - 10) % len(alphabet)], end="")

    if m_char == " ":
        print(" ", end="")

print()

message = "HVXBGDFCX"

for m_char in message:
    for index, a_char in enumerate(alphabet):
        if a_char == m_char:
            print(alphabet[(index + 12) % len(alphabet)], end="")
            
    if m_char == " ":
        print(" ", end="")
    
print()    
