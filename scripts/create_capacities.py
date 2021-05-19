output_filename = "./provided/toulon/capacities.txt"
cnt = 763
capacity = 30

output = open(output_filename, "w")

output.write("customer capacity\n")
for i in range(1, cnt + 1):
    line = str(i) + ' ' + str(capacity) + '\n'
    output.write(line)
