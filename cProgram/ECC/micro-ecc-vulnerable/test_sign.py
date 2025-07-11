from ecpy.curves import Curve, Point
from ecpy.keys import ECPublicKey, ECPrivateKey
from ecpy.ecdsa import ECDSA

msg = [1] * 64

curve = Curve.get_curve('secp256r1')

private_key_nr = 0x48775362b141bc1036fade0fe5b8d5b640ac23fe1608f81a13531e85fdf06ccf

private_key = ECPrivateKey(private_key_nr, curve)

public_key = private_key.get_public_key()

sign1 = [108, 196, 111, 255, 246, 255, 137, 39, 3, 249, 161, 221, 139, 185, 98, 103, 254, 243, 72, 58, 30, 26, 160, 52, 187, 9, 122, 231, 47, 195, 50, 123, 108, 70, 49, 22, 53, 132, 172, 121, 131, 223, 120, 226, 76, 213, 199, 38, 132, 9, 99, 241, 188, 241, 202, 159, 230, 23, 111, 10, 17, 190, 49, 165]
sign2 = [12, 144, 29, 66, 60, 131, 28, 168, 94, 39, 199, 60, 38, 59, 161, 50, 114, 27, 185, 215, 168, 76, 79, 3, 128, 178, 166, 117, 111, 214, 1, 51, 101, 170, 201, 151, 253, 137, 38, 230, 37, 162, 188, 92, 148, 252, 130, 52, 75, 198, 134, 147, 97, 85, 157, 114, 162, 213, 85, 214, 8, 153, 129, 189]


ecdsa = ECDSA('RAW')
is_valid = ecdsa.verify(msg, sign1, public_key)

print(is_valid)

ecdsa = ECDSA('RAW')
is_valid = ecdsa.verify(msg, sign2, public_key)

print(is_valid)

