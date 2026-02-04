# Kravspecifikation
1. Grænseflade - Grafik - Rød
2. Lagring - Disk - Grøn
3. Udveksling - Netværk - Blå

| Krav | Beskrivelse                                                                            | Prioritet |
|------|----------------------------------------------------------------------------------------|-----------|
| G1   | Grænsefladen skal vise en oversigt over alle noter                                     | 1         |
| G2   | Grænsefladen skal kunne oprette nye noter fra oversigten                               | 1         |
| G3   | Grænsefladen skal understøtte CommonMark-standarden                                    | 1         |
| G4   | Grænsefladen skal live-render Markdown ved linjeskift                                  | 1         |
| G5   | Grænsefladen skal kunne tilføje synkroniseringsenheder                                 | 1         |
| G6   | Grænsefladen skal kunne sortere noter i oversigten                                     | 2         |
| L1   | Noter skal lagres persistent                                                           | 1         |
| L2   | Noter skal automatisk gemmes                                                           | 1         |
| L3   | Konfigurerede synkroniseringsenheder skal lagres persistent                            | 1         |
| U1   | Alle noter i oversigten skal synkroniseres med de konfigurerede synkroniseringsenheder | 1         |
| U2   | Systemet skal håndtere offline-noter og synkronisere, når netværk er tilgængeligt      | 2         |
| U3   | Systemet skal håndtere konflikter ved samtidige ændringer                              | 2         |
