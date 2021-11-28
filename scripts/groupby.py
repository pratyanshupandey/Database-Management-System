import random
from collections import defaultdict

file = open("../data/GBY.csv", "w+")
col_count = 4
file.write("A, B, C, D\n")

row_cnt = int(input("Size: "))

data = []

for i in range(row_cnt):
    row = [random.randint(0, 50) for _ in range(col_count)]
    data.append(row)
    row = [str(ele) for ele in row]
    file.write(",".join(row) + "\n")

file.close()

# Max. min, sum, cnt

dictAopC = defaultdict(lambda: [-1, 1e10, 0, 0])
dictAopA = defaultdict(lambda: [-1, 1e10, 0, 0])

for row in data:
    A = row[0]
    C = row[2]

    res = dictAopC[A]
    res[0] = max(res[0], C)
    res[1] = min(res[1], C)
    res[2] += C
    res[3] += 1
    dictAopC[A] = res

    res = dictAopA[A]
    res[0] = max(res[0], A)
    res[1] = min(res[1], A)
    res[2] += A
    res[3] += 1
    dictAopA[A] = res



file_max = open("../data/GBYAAMAX.csv", "w+")
file_max.write("A, MAXA\n")

file_min = open("../data/GBYAAMIN.csv", "w+")
file_min.write("A, MINA\n")

file_sum = open("../data/GBYAASUM.csv", "w+")
file_sum.write("A, SUMA\n")

file_avg = open("../data/GBYAAAVG.csv", "w+")
file_avg.write("A, AVGA\n")

for key in sorted(dictAopA.keys()):
    val = dictAopA[key]
    file_max.write(str(key) + ", " + str(val[0]) + "\n")
    file_min.write(str(key) + ", " + str(val[1]) + "\n")
    file_sum.write(str(key) + ", " + str(val[2]) + "\n")
    file_avg.write(str(key) + ", " + str(int(val[2]/val[3])) + "\n")

file_max.close()
file_min.close()
file_sum.close()
file_avg.close()




file_max = open("../data/GBYACMAX.csv", "w+")
file_max.write("A, MAXC\n")

file_min = open("../data/GBYACMIN.csv", "w+")
file_min.write("A, MINC\n")

file_sum = open("../data/GBYACSUM.csv", "w+")
file_sum.write("A, SUMC\n")

file_avg = open("../data/GBYACAVG.csv", "w+")
file_avg.write("A, AVGC\n")

for key in sorted(dictAopC.keys()):
    val = dictAopC[key]
    file_max.write(str(key) + ", " + str(val[0]) + "\n")
    file_min.write(str(key) + ", " + str(val[1]) + "\n")
    file_sum.write(str(key) + ", " + str(val[2]) + "\n")
    file_avg.write(str(key) + ", " + str(int(val[2]/val[3])) + "\n")

file_max.close()
file_min.close()
file_sum.close()
file_avg.close()