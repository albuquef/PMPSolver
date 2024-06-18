using DataFrames
using CSV

include("instance.jl")


function basic_model(model,p,num_locations,num_customers)

    println("Creating the variables - x_ij")
    # @variable(model, x[i in df_customers.customer, j in df_locations.location], Bin);
    if model_bin
        @time @variable(model, x[i in 1:num_customers, j in 1:num_locations], Bin);
    else
        @time @variable(model, 0 <= x[i in 1:num_customers, j in 1:num_locations] <= 1);
    end


    println("Creating the variables - y_j")
    # @variable(model, y[j in size(df_locations.location)], Bin);
    @time @variable(model, y[j in 1:num_locations], Bin);

    println("Creating the objective function")
    # @time @objective(model, Min, sum(round.(cust_weights[i];digits=2)*getDist(i,j)*x[i, j] for i in 1:num_customers,j in 1:num_locations))
    @time @objective(model, Min, sum(round.(cust_weights[i];digits=5)*dist_matrix[i,j]*x[i, j] for i in 1:num_customers,j in 1:num_locations))

    println("Creating the constraint - Satisfied Customer")
    @time @constraint(model, customer_service[i in 1:num_customers],
        sum(x[i, j] for j in 1:num_locations) == 1
    );

    println("Creating the constraint - Exact p Locations")
    @time @constraint(model, p_locations,
        sum(y[j] for j in 1:num_locations) == p
    );

    return model, x,y

end

function create_model(problem, model,p,num_locations,num_customers)
   
    println("[INFO] Creating the model")

    model,x,y = basic_model(model,p,num_locations,num_customers)

    if problem == "PMP"
        println("Creating the constraint - customers attended only by p medians")
        @time @constraint(model, location_capacity[i in 1:num_customers,j in 1:num_locations], 
            x[i, j] <= y[j]
        );
    elseif problem == "CPMP"
        println("Creating the constraint - Capacity Limit")
        @time @constraint(model, location_capacity[j in 1:num_locations], 
            sum(round.(cust_weights[i];digits=5)*x[i, j] for i in 1:num_customers) <= loc_capacities[j] * y[j]
        );
    elseif problem == "PMP_SC"
        println("Creating the constraint - custormers attended only by p medians")
        @time @constraint(model, location_capacity[i in 1:num_customers,j in 1:num_locations], 
            x[i, j] <= y[j]
        );
        println("Creating the constraint - Subareas Coverage")
        @time @constraint(model, cover_subarea[k in 1:num_subareas], 
                sum(isInSubarea(j,k)*y[j] for j in 1:num_locations) >= isSubarea_available(k) # only if hava a possible facility in the subarea
                # sum(y[j] for j in 1:num_locations; isInSubarea(j,k) == 1) >= 1
        );
    elseif problem == "CPMP_SC"
        println("Creating the constraint - Capacity Limit")
        @time @constraint(model, location_capacity[j in 1:num_locations], 
            sum(round.(cust_weights[i];digits=2)*x[i, j] for i in 1:num_customers) <= loc_capacities[j] * y[j] 
        );
        println("Creating the constraint - Subareas Coverage")
        @time @constraint(model, cover_subarea[k in 1:num_subareas], 
                sum(isInSubarea(j,k)*y[j] for j in 1:num_locations) >= isSubarea_available(k) # only if hava a possible facility in the subarea
                # sum(y[j] for j in 1:num_locations; isInSubarea(j,k) == 1) >= 1
        );
    end

    return model,x,y

end

# not working yet - out of service
function create_model_CPMP_MC(model,p,num_locations,num_customers)
    
    model,x,y = create_model_CPMP(model,p,num_locations,num_customers)
    
    num_territorials = 2
    @time @variable(model, z[l in 1:num_territorials, k in 1:num_subareas], Bin);

    println("Creating the objective function with penalties") # to modify objective function 
    @time @objective(model, Min, sum(round.(cust_weights[i];digits=2)*dist_matrix[i,j]*x[i, j] for i in 1:num_customers for j in 1:num_locations))

    println("Creating the constraint - Scales Coverage") 
    @time @constraint(model, cover_subarea[k in 1:num_subareas], 
            sum(isInSubarea(j,k)*y[j] for j in 1:num_locations) >= isSubarea_available(k) 
            # sum(y[j] for j in 1:num_locations; isInSubarea(j,k) == 1) >= 1
    );

    # α = zeros(2,num_subareas) 
    α = []
    for l in num_territorials
        # push!(α,[])
        append!(α,[])
    end

    return model,x,y

end

function printResult_info(model,x,y)
    println("--------------------------------")
    println("\n Result Info")

    list_result=Int[]
    for j in 1:num_locations
        if value.(y[j]) >= 0.9
            push!(list_result, Int(j))
            # push!(list_result, ";")
        end
    end 
    println("p-locations: ")
    println(list_result)


    println("\nLOCATION CAPACITY")
    println("location:  utilized/limit")
    cap_used = 0
    cap_disp = 0
    cont_null = 0
    for j in list_result
        print(j)
        print(": ")
        cap = 0
        for i in 1:num_customers
            if value.(x[i,j]) == 1.0
                cap += round.(cust_weights[i];digits=2)
            end 
        end
        cap_used += cap
        if cap >= 0 cap_disp += loc_capacities[j] end
        if cap == 0
            cont_null += 1
        end 
        print(round.(cap;digits=2))
        print("/",loc_capacities[j])
        print("\n")
    end
    cap_residual = round.((cap_disp - cap_used);digits=2) 
    print("Remaining capacity:  $(round.(cap_residual;digits=2)) \n")
    print("Number of medians with 0 demand:  $cont_null \n")


    println("\nLOCATION ASSIGNMENTS")
    println("location: customers")
    for j in list_result
        print(j)
        print(": ")
        for i in 1:num_customers
            if value.(x[i,j]) == 1.0
                print(Int(i))
                print(" ")
            end 
        end
        print("\n")
    end

    println("\nLOCATION CUSTOMERS DISTANCES")
    println("location: customers (distance)")
    for j in list_result
        print(j)
        print(": ")
        for i in 1:num_customers
            if value.(x[i,j]) == 1.0
                print(Int(i))
                print(" (")
                print(dist_matrix[i,j])
                # print(dist_matrix[i,j])
                print(") ")
            end 
        end
        print("\n")
    end

    println("\nOptimal value: "*string.(round.(objective_value(model); digits=3)))
    println("Solve time cplex : "*string.(round.(solve_time(model); digits=3)))
    println("Solve time: "*string.(round.(time_to_solve; digits=3)))

end

function saveResult_info(model,x,y, FILE_NAME)
    println("--------------------------------")
    println("\n[INFO] Saving Result Info")

    if isdir(OUTPUT_PATH*"/Results/") == 0
        mkdir(OUTPUT_PATH*"/Results/")  # create the folder
    end

    DIR = OUTPUT_PATH*"/Results/"*string(num_customers)*"/"
    if isdir(DIR) == 0
        mkdir(DIR)  # create the folder
    end

    # FILE_NAME = DIR*"output_"*problem*"_"*string(num_customers)*"_"*string(num_locations)*"_p_"*string(p)*".txt"
    open(DIR*FILE_NAME,"w") do file
    
        list_result=Int[]
        for j in 1:num_locations
            if value.(y[j]) >= 0.9
                push!(list_result, Int(j))
                # push!(list_result, ";")
            end
        end 
    
        write(file,"p-locations: $list_result \n")
    
        write(file,"\nLOCATION CAPACITY\n")
        write(file,"location:  utilized/limit \n")
        cap_used = 0
        cap_disp = 0
        global vet_p_null = []
        global cont_null = 0
        for j in list_result
            write(file,"$j: ")
            cap = 0
            for i in 1:num_customers
                if value.(x[i,j]) == 1.0
                    cap += round.(cust_weights[i];digits=2)
                end 
            end
            write(file,"$(round.(cap;digits=2))/$(loc_capacities[j]) \n")
            cap_used += cap
            if cap > 0 cap_disp += loc_capacities[j] end
            if cap == 0
                cont_null += 1
                push!(vet_p_null, j)
            end 
        end
        
        global cap_residual = round.((cap_disp - cap_used);digits=2) 
        write(file,"Remaining capacity:  $(round.(cap_residual;digits=2)) \n")
        write(file,"Number of medians with 0 demand:  $cont_null \n")
        write(file,"Medians with 0 demand:  $vet_p_null \n")

        write(file,"\nLOCATION ASSIGNMENTS \n")
        write(file, "location: customers \n")
        for j in list_result
            write(file,"$j: ")
            for i in 1:num_customers
                if value.(x[i,j]) == 1.0
                    write(file,"$(Int(i)) ")
                end 
            end
            write(file,"\n")
        end
    
        write(file,"\nLOCATION CUSTOMERS DISTANCES\n")
        write(file,"location: customers (distance)\n")
        for j in list_result
            write(file,"$j: ")
            for i in 1:num_customers
                if value.(x[i,j]) == 1.0
                    write(file, "$(Int(i)) ($(dist_matrix[i,j]))")
                    # write(file, "$(Int(i)) ($(dist_matrix[i,j]))")
                end 
            end
            write(file,"\n")
        end
    
        write(file,"\nOptimal value: "*string.(round.(objective_value(model); digits=3)))
        write(file,"\nSolve time - CPLEX: "*string.(round.(solve_time(model); digits=3)))
        write(file,"\nSolve time: "*string.(round.(time_to_solve; digits=3)))
    
    end
end

function saveVarValues(model,x,y, FILE_NAME)

    println("--------------------------------")
    println("\n[INFO] Saving Outputs Var Values")
    
    if isdir(OUTPUT_PATH*"/Results/") == 0
        mkdir(OUTPUT_PATH*"/Results/")  # create the folder
    end

    DIR = OUTPUT_PATH*"/Results/"*string(num_customers)*"/"
    if isdir(DIR) == 0
        mkdir(DIR)  # create the folder
    end

    # FILE_NAME = DIR*"outputVarValues_"*problem*"_"*string(num_customers)*"_"*string(num_locations)*"_p_"*string(p)*"_"*typeSub*"_"*typeCap*".txt"
    
    # FILE_NAME = DIR*"outputVarValues_"*problem*"_"*string(num_customers)*"_"*string(num_locations)*"_p_"*string(p)*".txt"
    

    open(DIR*FILE_NAME,"w") do file
        
        write(file,"y\n")
        for j in 1:num_locations
            if value.(y[j]) > 1 - 1e-5
                write(file,"y($j) = 1\n")
            end 
        end

        write(file,"x\n")
        for j in 1:num_locations
            for i in 1:num_customers
                if value.(x[i,j]) > 1 - 1e-5
                    write(file, "x($i,$j) = 1\n")
                end 
            end
        end

    end

end

function saveResult_csv(model,x,y,filename)

    println("--------------------------------")
    println("\n[INFO] Saving Result Data csv")

    DIR = OUTPUT_PATH*"/Tables/"
    if isdir(OUTPUT_PATH*"/Tables/") == 0
        mkdir(OUTPUT_PATH*"/Tables/")  # create the folder
    end

    GAP = MOI.get(model, MOI.RelativeGap())
    # Num_nos = MOI.get(model, MOI.NodeCount())   
    Num_nos = "null"   

    df = DataFrame(type_problem = problem, num_cust = num_customers, num_loc = num_locations, num_p = p, 
                    opt_value = round.(objective_value(model); digits=3),gap =  round.(GAP; digits=3),Nnos=Num_nos,#round.(Num_nos; digits=3),
                    timeCplex = round.(solve_time(model); digits=3), time = round.(time_to_solve; digits=3)
                )

    CSV.write(DIR*filename*".csv", df, delim = ';',append=true)

    if !isempty(vet_subareas)
        df2 = DataFrame(typeProblem = problem, numCust = num_customers, numLoc = num_locations, num_p = p, num_p_null = cont_null, resCcap = cap_residual)
        for k = 1:num_subareas
            cont_j = 0
            for j in 1:num_locations
                if isInSubarea(j,k) == 1 && value.(y[j]) > 1 - 1e-5 #&& !(j in vetor_p_nul)
                    cont_j += 1
                end
            end
            colname = "S$k"
            df2[!, colname] = [cont_j]
        end

        CSV.write(DIR*filename*"_subareas.csv", df2, delim = ';',append=true)
    end
end

function saveResult_csv_PACA(model,x,y,filename)

    println("--------------------------------")
    println("\n[INFO] Saving Result Data csv")

    DIR = OUTPUT_PATH*"/Tables/"
    if isdir(OUTPUT_PATH*"/Tables/") == 0
        mkdir(OUTPUT_PATH*"/Tables/")  # create the folder
    end

    GAP = MOI.get(model, MOI.RelativeGap())
    # Num_nos = MOI.get(model, MOI.NodeCount())   
    Num_nos = "null"   

    df = DataFrame(type_problem = problem, num_cust = num_customers, num_loc = num_locations, num_p = p, subareas=typeDiv,
                    opt_value = round.(objective_value(model); digits=3),gap =  round.(GAP; digits=3),Nnos=Num_nos,#round.(Num_nos; digits=3),
                    timeCplex = round.(solve_time(model); digits=3), time = round.(time_to_solve; digits=3)
                )

    CSV.write(DIR*filename*".csv", df, delim = ';',append=true)

    df2 = DataFrame(typeProblem = problem, numCust = num_customers, numLoc = num_locations, num_p = p, num_p_null = cont_null, resCcap = cap_residual)
    for k = 1:num_subareas
        cont_j = 0
        for j in 1:num_locations
            if isInSubarea(j,k) == 1 && value.(y[j]) > 1 - 1e-5 #&& !(j in vetor_p_nul)
                cont_j += 1
            end
        end
        colname = "S$k"
        df2[!, colname] = [cont_j]
    end

    CSV.write(DIR*filename*"_subareas.csv", df2, delim = ';',append=true)

end



# function plotSolution(model,x,y)
    
#     println("--------------------------------")
#     println("\n[INFO] Plotting Solution: ")

#     DIR = "./output/Plots/"
#     if isdir(DIR) == 0
#         mkdir(DIR)  # create the folder
#     end

#     DIR = "./output/Random/Plots/"
#     if isdir(DIR) == 0
#         mkdir(DIR)  # create the folder
#     end

#     x_ = value.(x) .> 1 - 1e-5 #x[i, j] ≈ 1
#     y_ = value.(y) .> 1 - 1e-5

#     #customers
#     fig = plot()
#     # scatter!(fig, df_customers.Xc, df_customers.Yc, label=nothing,
#                 # markershape=:circle, markercolor=:blue, markersize= 2 .*(2 .+ df_customers.weight))
    
#     #[1][2]
#     #[3][4] subareas and points(4) follow this model (sequence) 

#     for Ci in C_sets_plot
#         plot!(fig, Shape([Ci[1].x, Ci[2].x, Ci[4].x,  Ci[3].x],
#                     [Ci[1].y, Ci[2].y, Ci[4].y, Ci[3].y]), 
#                     # opacity=.5,label="C"*string(cont), legend = :outertopright)
#                     label=nothing, color=nothing, markerstrokecolor=:black, linecolor=:black)
#     end

#     for Si in S_sets_plot
#         plot!(fig, Shape([Si[1].x, Si[2].x, Si[4].x,  Si[3].x], 
#                     [Si[1].y, Si[2].y, Si[4].y, Si[3].y]), 
#                     # opacity=.5,label="C"*string(cont), legend = :outertopright)
#                     # label=nothing, linecolor=:blue, color=nothing )#opacity=.1)
#                     label=nothing, linecolor=:black, opacity=.2)
#     end

#     # if type_instance == "MC"
#     for Sci in Scale_sets_plot
#         plot!(fig, Shape([Sci[1].x, Sci[2].x, Sci[4].x,  Sci[3].x], 
#                     [Sci[1].y, Sci[2].y, Sci[4].y, Sci[3].y]), 
#                     # opacity=.5,label="C"*string(cont), legend = :outertopright)
#                     # label=nothing, linecolor=:blue, color=nothing )#opacity=.1)
#                     # label=nothing, linecolor=:black, opacity=.2)
#                     label=nothing, linecolor=:purple, color=nothing, linewidth=3)
#     end
#     # end



#     #fill open facilities 
#     mc = [(y_[j] ? :red : :white) for j in 1:num_locations]
#     plot!(scatter!(fig,df_locations.Xf, df_locations.Yf, label=nothing, 
#                     # markershape=:rect, markercolor=mc, markersize=df_locations.capacity,
#                     # markerstrokecolor=:red, markerstrokewidth=2)
#                     markershape=:utriangle, markercolor=mc, markersize= df_locations.capacity,
#                     markerstrokecolor=:red,
#                     markerstrokewidth=2)
#                 )

#     # println(cont_p)
#     # Show client-facility assignment
#     for i in 1:num_customers
#         for j in 1:num_locations
#             if x_[i, j] == 1
#                 plot!(fig, [df_customers.Xc[i], df_locations.Xf[j]], [df_customers.Yc[i], df_locations.Yf[j]], color=:black, label=nothing)
#             end
#         end
#     end


#     if isdir("./plots/") == 0
#         mkdir("./plots/")  # create the folder
#     end

#     file_name = "solution_"*problem*"_"*string(num_customers)*"_"*string(num_locations)*"_p_"*string(p)*".pdf"
#     savefig(DIR*file_name)
#     println("Save solution: "*DIR*file_name)
# end