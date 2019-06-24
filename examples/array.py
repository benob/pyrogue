import rl
a = rl.Array(2, 2)
a.fill(2)
b = rl.Array(2, 2)
b.fill(4)

print('==== operators')
print('+', a + 2, 2 + a, a + b)
print('-', a - 3, 3 - a, a - b)
print('*', a * 2, 3 * a, a * b)
print('/', a / 2, 4 / a, a / b, b / a)
print('%', a % 3, 3 % a, a % b, b % a)
print('&', a & 3, 3 & a, a & b, b & a)
print('|', a | 3, 3 | a, a | b, b | a)
print('^', a ^ 3, 3 ^ a, a ^ b, b ^ a)
print('<<', a << 3, 3 << a, a << b, b << a)
print('>>', a >> 1, 3 >> a, a >> b, b >> a)
print('unary+', +a)
print('unary-', -a)
print('unary~', ~a)
print('unary bool', bool(a), bool(rl.Array(2, 2)))

print('==== inplace operators')
a += 5
print('+=', a)
a -= 5
print('-=', a)
a *= 5
print('*=', a)
a /= 2
print('/=', a)
a %= 3
print('%=', a)
a &= 3
print('&=', a)
a |= 3
print('|=', a)
a ^= 7
print('^=', a)
a.fill(1)
a <<= 1
print('<<=', a)
a >>= 1
print('>>=', a)

print('==== comparators')
#print(a == b) # not functionnal due to bug in micropython
#print(a != b)
print(a.equals(b))
print(a.not_equals(b))
print(a < b)
print(a >= b)
print(a > b)
print(a <= b)

print('==== example')
a = rl.Array(2, 2)
a.random_int(2, 6)
print(a)
b = a.equals(2)
print(b)
c = b * 2 + 3
print(c << 1)

print('==== from list')
a = rl.array_from_list([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
print(a)

print('==== view from slice')
v = a[:2, 1:]
print(v)
v[:,:] = -1
print(v)
print(a)
v[:,:] = rl.array_from_list([[-3, -2], [-2, -3]])
print(a)
print(a[:, -1]) # mixed indices: view on last row
