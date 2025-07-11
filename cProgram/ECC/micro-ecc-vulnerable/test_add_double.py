from ecpy.curves import Curve, Point
from ecpy.keys import ECPublicKey, ECPrivateKey
from ecpy.ecdsa import ECDSA

curve = Curve.get_curve('secp256r1')



private_key_nr = 0x48775362B141BC1036FADE0FE5B8D5B640AC23FE1608F81A13531E85FDF06C02

private_key = ECPrivateKey(private_key_nr, curve)

public_key = private_key.get_public_key()

print(public_key)

# ecdsa = ECDSA('RAW')

# G = curve.generator

# with open('/home/sjors/development/ce2/hardware/victims/firmware/crypto/micro-ecc-vulnerable/testdata.txt', 'r') as fp:
#     lines = fp.readlines()

# R = Point.infinity()
# count = 0
# for line in lines:
#     cmd,num = line.split(' ')
#     num = int(num)
#     xval = 0

#     if cmd == 'double':
#         R = 2*R
#         if not R.is_infinity:
#             xval = int.from_bytes(R.x.to_bytes(32, 'big')[28:32], byteorder='big')
#         assert num == xval, f'DOUBLE - num was not equal at count={count}\n{hex(xval)} != {hex(num)}'
#     elif cmd == 'add':
#         R = R + G
#         if not R.is_infinity:
#             xval = int.from_bytes(R.x.to_bytes(32, 'big')[28:32], byteorder='big')
#         assert num == xval, f'ADD - num was not equal at count={count} \n{hex(xval)} != {hex(num)}'

#     print(f'{count} numbers were equal: {hex(xval)} == {hex(num)}', flush=True)
#     count += 1
