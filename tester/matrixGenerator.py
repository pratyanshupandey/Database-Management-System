import random

N = int(input("Matrix Size: "))

file = open("../data/MAT" + str(N) + ".csv", "w+")

for i in range(N):
    for j in range(N):
        ele = random.randrange(1,1000000)
        file.write(str(ele))
        if j == N-1:
            file.write("\n")
        else:
            file.write(",")

file.close()