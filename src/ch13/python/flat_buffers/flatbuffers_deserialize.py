#!/usr/bin/env python3

from pathlib import Path
from MyGame.Monster import Monster


with open(Path(__file__).parent.parent.parent / 'flatbuffers' / 'monster.bin', 'rb') as f:
    monster = Monster.GetRootAsMonster(bytearray(f.read()), 0)

    hp = monster.Hp()
    name = monster.Name()

    print(name, hp)
