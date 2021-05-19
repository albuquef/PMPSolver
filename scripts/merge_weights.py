# customers = './provided/toulon/customers.txt'
# weights = './provided/toulon/weights.txt'
# output = './provided/toulon/weights_labeled.txt'

# customers = './provided/paca/customers.txt'
# weights = './provided/paca/weights.txt'
# output = './provided/paca/weights_labeled.txt'

weights = './provided/matdistpaca_600_1500_tps_90/weights.txt'
output = './provided/matdistpaca_600_1500_tps_90/weights_labeled.txt'

f_weights = open(weights, "r")
f_out = open(output, "w")

cust = 1
while True:
    weight = f_weights.readline()
    if not weight:
        break
    string = str(cust) + " " + weight.rstrip() + "\n"
    f_out.write(string)
    cust += 1
