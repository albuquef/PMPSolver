using DelimitedFiles

struct Point
    x::Float64
    y::Float64
end

UB_DIST = 1000000000

function readCustomers(PATH,num_customers,filename)
    println("[INFO] Loading Customers data")

    cust_weights = zeros(num_customers)

    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        nrows = size(reader,1)
        for i in 2:nrows  #first line == collumns names
            line = reader[i,:] 
            cust_weights[line[1]] = line[2]
        end
    else
        println("No customers file exists")
        return 0
    end
    
    return cust_weights
end

function readCustomers_2(num_customers,PATH,filename)
    println("[INFO] Loading Customers data")

    cust_weights = zeros(num_customers)

    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        nrows = size(reader,1)
        for i in 2:nrows  #first line == collumns names
            line = reader[i,:] 
            cust_weights[line[1]] = line[2]
        end
    else
        println("No customers file exists")
        return 0
    end
    
    return cust_weights
end


function readCustomers_df(PATH,filename)
    println("[INFO] Loading Customers data df")
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        header, data = reader[1,:], reader[2:end,:]
        df = DataFrames.DataFrame(data, header)
        df[!,:customer] = convert.(Int,df[!,:customer])
        df[!,:weight] = convert.(Float64,df[!,:weight])
        df[!,:Xc] = convert.(Float64,df[!,:Xc])
        df[!,:Yc] = convert.(Float64,df[!,:Yc])
        # println(first(df, 4))
    else
        println("No customers file exists")
        return 0
    end
    
    return df
end

function readCustomers_coord_df(PATH,filename)
    println("[INFO] Loading Customers data df")
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        header, data = reader[1,:], reader[2:end,:]
        df = DataFrames.DataFrame(data, header)
        df[!,:customer] = convert.(Int,df[!,:customer])
        df[!,:Xc] = convert.(Float64,df[!,:Xc])
        df[!,:Yc] = convert.(Float64,df[!,:Yc])
        # println(first(df, 4))
    else
        println("No customers file exists")
        return 0
    end
    
    return df
end


function readLocations(PATH,num_locations,filename)
    println("[INFO] Loading Locations data "*filename)

    loc_capacities = zeros(num_locations)

    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        nrows = size(reader,1)
        for i in 2:nrows  #first line == collumns names
            line = reader[i,:] 
            if line[1] <= num_locations
                loc_capacities[line[1]] = line[2]
            end
        end
    else
        println("No locations file exists")
        return 0
    end

    return loc_capacities


end

function readLocations_df(PATH,num_locations,filename)
    println("[INFO] Loading Locations data df")
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        header, data = reader[1,:], reader[2:end,:]
        
        df = DataFrames.DataFrame(data, header)
        df[!,:location] = convert.(Int,df[!,:location])
        df[!,:capacity] = convert.(Float64,df[!,:capacity])
        df[!,:Xf] = convert.(Float64,df[!,:Xf])
        df[!,:Yf] = convert.(Float64,df[!,:Yf])
        # println(first(df, 4))
    else
        println("No locations file exists")
        return 0
    end 

    return first(df, num_locations)
end

function readLocations_df_2(PATH)
    println("[INFO] Loading Locations data df")
    if isfile(PATH*"loc_capacities_2x2_ratio.txt")
        # reader = readdlm(PATH*"loc_capacities_2x2_ratio.txt")
        reader = readdlm(PATH*"loc_capacities_2x2_ratio.txt")
        header, data = reader[1,:], reader[2:end,:]
        
        df = DataFrames.DataFrame(data, header)
        df[!,:location] = convert.(Int,df[!,:location])
        df[!,:capacity] = convert.(Float64,df[!,:capacity])
        df[!,:Xf] = convert.(Float64,df[!,:Xf])
        df[!,:Yf] = convert.(Float64,df[!,:Yf])
        # println(first(df, 4))
    else
        println("No locations file exists")
        return 0
    end 

    return df
end

function readLocations_df_3(PATH,num_locations,filename)
    println("[INFO] Loading Locations data df")
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        header, data = reader[1,:], reader[2:end,:]
        
        df = DataFrames.DataFrame(data, header)
        df[!,:location] = convert.(Int,df[!,:location])
        df[!,:capacity] = convert.(Float64,df[!,:capacity])
        df[!,:Xf] = convert.(Float64,df[!,:Xf])
        df[!,:Yf] = convert.(Float64,df[!,:Yf])
        # println(first(df, 4))
    else
        println("No locations file exists")
        return 0
    end 

    return first(df, num_locations)
end

function readLocations_coord_df(PATH,num_locations,filename)
    println("[INFO] Loading Locations Coordenates df")
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        header, data = reader[1,:], reader[2:end,:]
        df = DataFrames.DataFrame(data, header)
        df[!,:location] = convert.(Int,df[!,:location])
        df[!,:Xf] = convert.(Float64,df[!,:Xf])
        df[!,:Yf] = convert.(Float64,df[!,:Yf])
    else
        println("No locations coord file exists")
        return 0
    end
    
    return first(df, num_locations)
end


function readDistances(PATH,filename)
    println("[INFO] Loading Distances matrix df")
    # matrix = UB_DIST*ones(num_customers,max_locations)
    matrix = UB_DIST*ones(num_customers,num_locations)
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        nrows = size(reader,1)
        for i in 2:nrows  #first line == collumns names
            line = reader[i,:] 
            if line[2] <= num_locations
                matrix[line[1],line[2]] = line[3]
            end
        end
    else
        println("No distance matrix file exists")
        return 0
    end
    return matrix
end

function readDistances_df(PATH)
    println("[INFO] Loading Distances matrix df")
    if isfile(PATH*"dist_matrix.txt")
        reader = readdlm(PATH*"dist_matrix.txt")
        header, data = reader[1,:], reader[2:end,:]
        df = DataFrames.DataFrame(data[:,1:3], header[1:3])
        df[!,:customer] = convert.(Int,df[!,:customer])
        df[!,:location] = convert.(Int,df[!,:location])
        df[!,:distance] = convert.(Float64,df[!,:distance])
        # println(first(df, 4))
    else
        println("No distance matrix file exists")
        return 0
    end

    return df
end

function readSubareas(PATH,filename)
    println("[INFO] Loading Locations Coverages "*filename)

    vet_subareas = -1*ones(num_locations)
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        nrows = size(reader,1)
        for i in 2:nrows  #first line == collumns names
            line = reader[i,:] 
            if line[1] <= num_locations
                vet_subareas[line[1]] = line[2]
            end
        end
    else
        println("No locations coverage file exists")
        return 0
    end
    
    return vet_subareas
end

function readSubareas_df(PATH,filename)
    println("[INFO] Loading Locations Coverages df")
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        header, data = reader[1,:], reader[2:end,:]
        df = DataFrames.DataFrame(data, header)
        df[!,:location] = convert.(Int,df[!,:location])
        df[!,:subarea] = convert.(Float64,df[!,:subarea])
        # println(first(df, 4))
    else
        println("No locations coverage file exists")
        return 0
    end
    return first(df, num_locations)
end

function readCustomers_plot(PATH,filename) #"cust_plot.txt"
    
    println("[INFO] Reading Customers Plot")
    C_sets = []
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        if !isempty(reader)
            for i in 2:num_customers+1
                p1 = Point(reader[i,2], reader[i,3])
                p2 = Point(reader[i,4], reader[i,5])
                p3 = Point(reader[i,6], reader[i,7])
                p4 = Point(reader[i,8], reader[i,9])
                push!(C_sets,[p1,p2,p3,p4]) 
            end
        else
            println("Plot customers is empty")
        end
    else 
        println("No file to plot customers")
    end
        
    return C_sets

end

function readSubareas_plot(PATH,filename)

    println("[INFO] Loading Subareas Plot "*filename)
    S_sets = []
    if isfile(PATH*filename)
        reader = readdlm(PATH*filename)
        size =floor(Int,length(reader)/9) # 9 columns in tx
        if !isempty(reader)
            for i in 2:size
                p1 = Point(reader[i,2], reader[i,3])
                p2 = Point(reader[i,4], reader[i,5])
                p3 = Point(reader[i,6], reader[i,7])
                p4 = Point(reader[i,8], reader[i,9])
                push!(S_sets,[p1,p2,p3,p4])
            end
        else
            println("plot subareas is empty")
        end
    else
        println("No file to plot customers")
    end
    
    return S_sets

end

function getCapacity(LOC)
    capacity = 0
    a = df_locations[df_locations.location .==LOC, :].capacity
    capacity = convert(Float64,a[1])
    return round.(capacity;digits=2)
end

function getCustomerWeight(CUST)
    weight = 0 
    a = df_customers[df_customers.customer .==CUST, :].weight
    weight = convert(Float64,a[1])
    return round.(weight; digits = 2)
    # return 1 # to compare heuristic
end

function getDist(CUST,LOC)
    dist = 0
    idx = df_distances[(df_distances.customer .==CUST) .& (df_distances.location .== LOC), :].distance
    if (isempty(idx)) # If there is no edge
        dist = UB_DIST
    else
        dist = convert(Float64,idx[1])
    end

    return dist
end

function isInSubarea(LOC,Subarea)
    if (vet_subareas[LOC] == Subarea)
        return 1
    else
        return 0
    end
end

function inSubarea(LOC,Subarea)
    idx = df_coverages[(df_coverages.location .==LOC) .& (df_coverages.subarea .== Subarea), :]
    if (isempty(idx))
        return 0
    else
        return 1
    end
end

function isSubarea_available(Subarea)
    for j in 1:num_locations
        if vet_subareas[j] == Subarea
            return 1 
        end
    end

    return 0
end

function available_subarea(Subarea)
    idx = df_coverages[(df_coverages.subarea .== Subarea), :]
    if (isempty(idx))
        return 0
    else
        return 1
    end
end


