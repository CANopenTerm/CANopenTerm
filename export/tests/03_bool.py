# test == !=
assert True == True
assert True != False
assert False == False
assert False != True

# test and/or/not
assert True and True
assert not (True and False)
assert True or True
assert True or False
assert not False
assert not (not True)

assert bool() == False
assert bool(0) == False
assert bool(1) == True
assert bool([]) == False
assert bool("abc") == True
assert bool([1,2]) == True
assert bool('') == False

# is operator
assert None == None
assert None is None
assert ... == ...
assert ... is ...
assert NotImplemented == NotImplemented
assert NotImplemented is NotImplemented

assert True is True
assert False is False

