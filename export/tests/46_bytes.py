a = '12345'
assert a.encode() == b'12345'

# test add
assert b'123' + b'456' == b'123456'
assert b'' + b'123' == b'123'
assert b'123' + b'' == b'123'
assert b'' + b'' == b''

assert b'\xff\xee' != b'1234'
assert b'\xff\xee' == b'\xff\xee'
assert len(b'\xff\xee') == 2
assert len(b'') == 0

a = '测试123'
assert a == a.encode().decode()

assert chr(0) == '\x00'
assert ord('\x00') == 0

# test slice
s = b"football"
q = b"abcd"
r = b"zoo"
t = b"this is string example....wow!!!"
assert s[0] == ord('f')
assert s[1:4] == b'oot'
assert s[:-1] == b'footbal'
assert s[:10] == b'football'
assert s[-3] == ord('a')
assert t[-5:] == b'ow!!!'
assert t[3:-3] == b's is string example....wow'

a = b"Hello, World!"
assert a[::-1] == b"!dlroW ,olleH"
assert a[::2] == b"Hlo ol!"
assert a[2:5:2] == b"lo"
assert a[5:2:-1] == b",ol"
assert a[5:2:-2] == b",l"

assert bytes() == b''
assert bytes((65,)) == b'A'
assert bytes([0, 1, 2, 3]) == b'\x00\x01\x02\x03'