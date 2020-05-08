import pypopminer

b = pypopminer.entities.BtcBlock()
b.version = 5
b.bits = 1337

h = b.toHex()

print(h)

z = pypopminer.entities.BtcBlock_fromHex(h)

print(z.bits)