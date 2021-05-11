# customers = './provided/toulon/customers.txt'
# locations = './provided/toulon/locations.txt'
# distances = './provided/toulon/distances.txt'
# w_distances = './provided/toulon/weighted_distances.txt'
# output = './provided/toulon/dist_matrix.txt'

customers = './provided/paca/customers.txt'
locations = './provided/paca/locations.txt'
distances = './provided/paca/distances.txt'
w_distances = './provided/paca/weighted_distances.txt'
output = './provided/paca/dist_matrix.txt'

f_cust = open(customers, "r")
f_loc = open(locations, "r")
f_dist = open(distances, "r")
f_w_dist = open(w_distances, "r")
f_out = open(output, "w")

while True:
    cust = f_cust.readline()
    loc = f_loc.readline()
    dist = f_dist.readline()
    w_dist = f_w_dist.readline()
    if not cust:
        break
    string = cust.rstrip() + " " + loc.rstrip() + " " + dist.rstrip() + " " + w_dist.rstrip() + "\n"
    f_out.write(string)
