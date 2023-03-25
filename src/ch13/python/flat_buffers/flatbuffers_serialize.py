#!/usr/bin/env python3


import flatbuffers
from MyGame import Color, Monster, Vec3, Weapon

# mypy: disable-error-code = attr-defined
builder = flatbuffers.Builder(0)

sword_name = builder.CreateString('Sword')
Weapon.WeaponStart(builder)
Weapon.WeaponAddName(builder, sword_name)
Weapon.WeaponAddDamage(builder, 3)
sword = Weapon.WeaponEnd(builder)

# Monster.Monster.StartWeaponsVector(builder, 2)
# Note: Since we prepend the data, prepend the weapons in reverse order.
# builder.PrependUOffsetTRelative(axe)
# builder.PrependUOffsetTRelative(sword)
# weapons = builder.EndVector()

Monster.MonsterStartInventoryVector(builder, 10)
# Note: Since we prepend the bytes, this loop iterates in reverse order.
for i in reversed(range(0, 10)):
    builder.PrependByte(i)

inv = builder.EndVector()

monster_name = builder.CreateString('Nessi')

Monster.Start(builder)
Monster.AddPos(builder, Vec3.CreateVec3(builder, 1.0, 2.0, 3.0))
Monster.AddHp(builder, 300)
Monster.AddName(builder, monster_name)
Monster.AddInventory(builder, inv)
Monster.AddColor(builder, Color.Color().Red)

# Object Id.
nessi_id = Monster.End(builder)
builder.Finish(nessi_id)

nessi = builder.Output()

print(Monster.Monster.GetRootAsMonster(nessi).Name())
