f = open("mat10.txt", "r")
f2 = f.read()
f3 = f2.replace("[", '{')
f4 = f3.replace("]", '}')
for i in range(len(f4)):
    if(next(f4) != "}" and next(f4) != "{"):
        f5 = f4.replace(".", ".,")

print(f5)