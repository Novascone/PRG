f = open("mat10(1).txt", "r")
f2 = f.read()
f3 = f2.replace("[", '{')
f4 = f3.replace("]", '}')
f5 = f4.replace(".", ".,")
f6 = f5.replace(",}" ,"}")
f7 = f6.replace("}}", "}};")
ff = open("mat.txt", "w")
ff.write(f7)