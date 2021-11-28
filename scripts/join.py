from os import read
import random
from collections import defaultdict

file = open("../data/JN1.csv", "w+")
col_count = 4
file.write("A, B, C, D\n")

row_cnt = int(input("Size Relation 1: "))

data1 = []

for i in range(row_cnt):
    row = [random.randint(0, 2000) for _ in range(col_count)]
    data1.append(row)
    row = [str(ele) for ele in row]
    file.write(", ".join(row) + "\n")

file.close()



file = open("../data/JN2.csv", "w+")
col_count = 5
file.write("E, F, G, H, I\n")

row_cnt = int(input("Size Relation 2: "))

data2 = []

for i in range(row_cnt):
    row = [random.randint(0, 2000) for _ in range(col_count)]
    data2.append(row)
    row = [str(ele) for ele in row]
    file.write(", ".join(row) + "\n")

file.close()


# B of R1 with I of R2


r1r2 = []
r2r1 = []

for row1 in data1:
    for row2 in data2:
        if row1[1] == row2[4]:

            new_row = row1[:]
            new_row.extend(row2)
            r1r2.append(new_row)

            new_row = row2[:]
            new_row.extend(row1)
            r2r1.append(new_row)

r1r2.sort()
r2r1.sort()

if r1r2 == []:
    print("12 NULL")

if r2r1 == []:
    print("21 NULL")

input("Complete and ENTER")

resr1r2 = []
file = open("../data/R1R2.csv")
file.readline()
while True:
    line = file.readline()
    if line == "":
        break
    row = [int(ele.strip()) for ele in line.strip().split(",")]
    resr1r2.append(row)
resr1r2.sort()



resr2r1 = []
file = open("../data/R2R1.csv")
file.readline()
while True:
    line = file.readline()
    if line == "":
        break
    row = [int(ele.strip()) for ele in line.strip().split(",")]
    resr2r1.append(row)
resr2r1.sort()


resr1r2h = []
file = open("../data/R1R2H.csv")
file.readline()
while True:
    line = file.readline()
    if line == "":
        break
    row = [int(ele.strip()) for ele in line.strip().split(",")]
    resr1r2h.append(row)
resr1r2h.sort()



resr2r1h = []
file = open("../data/R2R1H.csv")
file.readline()
while True:
    line = file.readline()
    if line == "":
        break
    row = [int(ele.strip()) for ele in line.strip().split(",")]
    resr2r1h.append(row)
resr2r1h.sort()


# for ele in data1:
#     print(ele)

# print("\n")

# for ele in data2:
#     print(ele)



if r1r2 != resr1r2:
    print("R1R2 diff")
    print("Py", len(r1r2))
    print("Code", len(resr1r2))
    

if r2r1 != resr2r1:
    print("R2R1 diff")
    print("Py", len(r2r1))
    print("Code", len(resr2r1))


if r1r2 != resr1r2h:
    print("R1R2H diff")
    print("Py", len(r1r2))
    print("Code", len(resr1r2h))
    
if r2r1 != resr2r1h:
    print("R2R1 diff")
    print("Py", len(r2r1))
    print("Code", len(resr2r1h))
