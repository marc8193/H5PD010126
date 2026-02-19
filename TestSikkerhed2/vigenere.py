#!/bin/env python3

alphabet = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
            "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Æ", "Ø", "Å"]

message = "HEJSA"
key = "TECHCOLLEGE"

encrypted_message = "XV FLT DYLVZ TTYUL"

# for index, m_char in enumerate(message):
#     m_char_alphabet_index = alphabet.index(m_char)
#     k_char_alphabet_index = alphabet.index(key[index % len(key)])

#     encrypted_message += alphabet[(k_char_alphabet_index + m_char_alphabet_index) % len(alphabet)]

# print(encrypted_message)

spaces = 0
decrypted_message = ""

for index, m_char in enumerate(encrypted_message):
    if m_char == " ":
        print(" ", end="")
        spaces += 1
        continue
    
    m_char_alphabet_index = alphabet.index(m_char)
    k_char_alphabet_index = alphabet.index(key[(index - spaces) % len(key)])

    print(alphabet[(m_char_alphabet_index - k_char_alphabet_index) % len(alphabet)], end="")

print()
