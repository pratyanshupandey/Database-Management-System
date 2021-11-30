import random
from functools import cmp_to_key

file = open("../data/REL.csv", "w+")
col_count = 7
cols = ["A", "B", "C", "D", "E", "F", "G"]

file.write(", ".join(cols) + "\n")

row_cnt = int(input("Size: "))

data = []
col_data = []

for _ in range(col_count):
    col_data.append(random.sample(range(-1000000, 1000000), row_cnt))

for i in range(row_cnt):
    row = [col_data[k][i] for k in range(col_count)]
    data.append(row)
    row = [str(ele) for ele in row]
    file.write(", ".join(row) + "\n")

file.close()


data_asc = []
for col in range(col_count):
    temp = sorted(data, key=cmp_to_key(lambda item1, item2: item1[col] - item2[col]))
    data_asc.append(temp)


data_desc = []
for col in range(col_count):
    temp = sorted(data, key=cmp_to_key(lambda item1, item2: item2[col] - item1[col]))
    data_desc.append(temp)

input("Process and Press ENTER")


def get_data(file_name):
    ret = []
    file = open("../data/" + file_name + ".csv")
    file.readline()
    while True:
        line = file.readline()
        if line == "":
            break
        row = [int(ele.strip()) for ele in line.strip().split(",")]
        ret.append(row)
    return ret


res_data_asc = []
for col in cols:
    res_data_asc.append(get_data("RES" + col + "A"))

res_data_desc = []
for col in cols:
    res_data_desc.append(get_data("RES" + col + "D"))


for col in range(col_count):

	if res_data_asc != data_asc:
	    	print("ASC diff", col)

	if res_data_desc != data_desc:
	    print("DESC diff", col)

print("Done")
