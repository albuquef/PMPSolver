# customers = './provided/toulon/customers.txt'
# weights = './provided/toulon/weights.txt'
# output = './provided/toulon/weights_labeled.txt'

customers = './provided/paca/customers.txt'
weights = './provided/paca/weights.txt'
output = './provided/paca/weights_labeled.txt'

f_cust = open(customers, "r")
f_weights = open(weights, "r")
f_out = open(output, "w")

# cust_prev = -1
# while True:
#     cust = f_cust.readline()
#     if not cust:
#         break
#     if cust != cust_prev:
#         weight = f_weights.readline()
#         cust_prev = cust
#         string = cust.rstrip() + " " + weight.rstrip() + "\n"
#         f_out.write(string)

cust = 1
while True:
    weight = f_weights.readline()
    if not weight:
        break
    string = str(cust) + " " + weight.rstrip() + "\n"
    f_out.write(string)
    cust += 1
