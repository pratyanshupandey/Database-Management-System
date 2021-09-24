import random

N = int(input("Matrix Size: "))

file = open("../data/MAT" + str(N) + ".csv", "w+")
filec = open("../data/MAT" + str(N) + "C" + ".csv", "w+")

matrix = []

for i in range(N):
    matrix.append([])
    for j in range(N):
        ele = random.randrange(1,1000000)
        matrix[-1].append(ele)

        file.write(str(ele))
        filec.write(str(ele))
        if j == N-1:
            file.write("\n")
            filec.write("\n")
        else:
            file.write(",")
            filec.write(",")

file.close()
filec.close()

for i in range(N-1):
    for j in range(i+1, N):
        temp = matrix[i][j]
        matrix[i][j] = matrix[j][i]
        matrix[j][i] = temp
        

filet = open("../data/MAT" + str(N) + "T" + ".csv", "w+")

for i in range(N):
    for j in range(N):
        ele = matrix[i][j]
        filet.write(str(ele))
        if j == N-1:
            filet.write("\n")
        else:
            filet.write(",")


filet.close()