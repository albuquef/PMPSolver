using Dates, Plots, JuMP
using Cbc, CPLEX
using DataFrames

include("pmp.jl")
include("instance.jl")

function solve(solver="CPLEX",saveReports=false,plotSol=false)
    
    global OUTPUT_PATH = "./output"
    # global OUTPUT_PATH = "./output/output_test_23_02_17/"
    # global OUTPUT_PATH = "./src_jl/output_jl/"

    if isdir(OUTPUT_PATH) == 0
        mkdir(OUTPUT_PATH)  # create the folder
    end


    # #----------- parameters: -----------
    println("-------- Solve --------------")
    println("Problem: ", problem)
    # println("Solver: ", solver)
    println("-------- Parameters --------------")
    println("Number of customers: ", num_customers)
    println("Number of locations: ", num_locations)
    println("Number of p medians: ", p)
    # println("Subareas: ", typeSub)
    println("----------------------------------")
    
    if solver == "HiGHS"
        model = Model(HiGHS.Optimizer)
    elseif solver == "GLPK"
        model = Model(GLPK.Optimizer)
        # set_optimizer_attribute(model, "tm_lim", 60 * 1_000)
        # set_optimizer_attribute(model, "msg_lev", GLPK.GLP_MSG_OFF)
    elseif solver == "Cbc"
        model = Model(Cbc.Optimizer)
    elseif  solver == "CPLEX"
        model = Model(CPLEX.Optimizer)
        set_optimizer_attribute(model, "CPX_PARAM_TILIM", 3600)	
        # set_optimizer_attribute(model, "CPX_PARAM_EPINT", 1e-8)
    end
    
    println("Current time: ",Dates.format(now(), "HH:MM")) 
    
    @time model,x,y = create_model(problem, model,p,num_locations,num_customers)

    write_to_file(model, "./model.lp")

    println("\n[INFO] Optimizing the Model")
    global time_to_solve = @elapsed begin  
        @time optimize!(model)
    end

    if has_values(model)
        printResult_info(model,x,y)

        if saveReports

            println("Saving VarValues and Results")

            namefile = problem*"_"*string(num_customers)*"_"*string(num_locations)*"_p_"*string(p)*"_"*typeSub*"_"*typeCap*".txt"

            @time saveVarValues(model,x,y,"outputVarValues_"*namefile)
            @time saveResult_info(model,x,y,"output_"*namefile)
            @time saveResult_csv(model,x,y,"results_"*string(num_customers))
        end

        if plotSol @time plotSolution(model,x,y) end
        
    else
        println(termination_status(model))
        println(solution_summary(model))
    end
    
    model = nothing #free memory

    println("-------------------------------------------")
end

function solve_PACA()
    

    # #----------- parameters: -----------
    println("-------- Solve PACA --------------")
    println("Problem: ", problem)
    # println("Solver: ", solver)
    println("-------- Parameters --------------")
    println("Number of customers: ", num_customers)
    println("Number of locations: ", num_locations)
    println("Number of p medians: ", p)
    println("Service: ", typeServ)
    println("Territorial division: ", typeDiv)
    println("Distance unit: ", typeDist)
    println("----------------------------------")
    
    if solver == "HiGHS"
        model = Model(HiGHS.Optimizer)
    elseif solver == "GLPK"
        model = Model(GLPK.Optimizer)
        # set_optimizer_attribute(model, "tm_lim", 60 * 1_000)
        # set_optimizer_attribute(model, "msg_lev", GLPK.GLP_MSG_OFF)
    elseif solver == "Cbc"
        model = Model(Cbc.Optimizer)
    elseif  solver == "CPLEX"
        model = Model(CPLEX.Optimizer)
        # set_optimizer_attribute(model, "CPX_PARAM_TILIM", 3600)	
        # set_optimizer_attribute(model, "CPX_PARAM_EPINT", 1e-8)
    end
    
    println("Current time: ",Dates.format(now(), "HH:MM")) 
    
    @time model,x,y = create_model(problem, model,p,num_locations,num_customers)



    println("\n[INFO] Optimizing the Model")
    global time_to_solve = @elapsed begin  
        @time optimize!(model)
    end

    if has_values(model)
        #printResult_info(model,x,y)
        if problem == "CPMP"
            namefile = problem*"_"*string(num_customers)*"_"*string(num_locations)*"_p_"*string(p)*"_"*typeServ*"_"*typeDist*".txt"
        else
            namefile = problem*"_"*string(num_customers)*"_"*string(num_locations)*"_p_"*string(p)*"_"*typeDiv*"_"*typeServ*"_"*typeDist*".txt"
        end
        @time saveVarValues(model,x,y,"outputVarValues_PACA_"*namefile)
        @time saveResult_info(model,x,y,"output_PACA_"*namefile)
        @time saveResult_csv(model,x,y,"results_PACA_"*typeServ*"_"*typeDist) # csv file
    else
        println(termination_status(model))
        println(solution_summary(model))
    end
    
    model = nothing #free memory

    println("-------------------------------------------")
end

function runInstances(typeSub,vet_grid,vet_problems)
    
    for grid in vet_grid

        global num_customers = grid*grid
        global max_locations = ceil(Int,1.5*c)

        if typeSub == "2x2" global num_subareas = ceil(Int,num_customers/4) end
        if typeSub == "4x4" global num_subareas = ceil(Int,num_customers/16) end

        vet_numLocations = [ceil(Int,i) for i in [0.5*num_customers,num_customers,1.5*num_customers]]
        vet_p = [ceil(Int,i) for i in [num_subareas,1.25*num_subareas,1.5*num_subareas]]


        for prob in vet_problems, loc in vet_numLocations, p_median in vet_p            

            println("\n\n--------------------------------------------------------------------------------------------\n\n")
            #parameters
            global problem = prob 
            global num_locations = loc
            global p = p_median                

            #---------------------------------------------
            #loading data
            global PATH_TO_DATA="./data/Random/random_"*string(num_customers)*"_"*string(max_locations)*"/"

            @time global cust_weights = readCustomers(PATH_TO_DATA,"cust_weights.txt")
            @time global loc_capacities = readLocations(PATH_TO_DATA,"loc_capacities.txt")
            @time global dist_matrix = readDistances(PATH_TO_DATA,"dist_matrix.txt")
            if typeSub == "2x2" @time global vet_subareas = readSubareas(PATH_TO_DATA,"loc_coverages.txt") end
            if typeSub == "4x4" @time global vet_subareas = readSubareas(PATH_TO_DATA,"loc_coverages_scale.txt") end
            @time global vet_scales = readSubareas(PATH_TO_DATA,"loc_coverages_scale.txt")
            # @time C_sets_plot,S_sets_plot,Scale_sets_plot = Instance_plot_reader(PATH_TO_DATA)

            solve(false)
            
        end
    end
end


function create_JobsList(vet_grid,vet_problems,vet_subareas_size)

    PATH = "./testList/"
    cont = 0
    for grid in vet_grid 
        FILE = "test_"*string(ceil(Int,grid*grid))*".txt"  

        open(PATH*FILE,"w") do file
            for subsize in vet_subareas_size
                typeSub = string(subsize)*"x"*string(subsize)

                for typeCap in ["rand", "ratio"]    
                    

                    c = grid*grid
                    vet_numLocations = [ceil(Int,i) for i in [0.5*c,c,1.5*c]]
                    size_S = c/(subsize*subsize) 
                    vet_p = [ceil(Int,i) for i in [size_S,1.25*size_S,1.5*size_S]]

                    typeSub = string(subsize)*"x"*string(subsize)

                    for prob in vet_problems
                        for loc in vet_numLocations
                            for p_median in vet_p  

                                if prob == "PMP" &&  typeCap == "rand" break end
                                if prob == "PMP_SC" &&  typeCap == "rand" break end
                                
                                write(file, "$prob $c $loc $p_median $typeSub $typeCap\n")
                                cont+=1
                            end
                        end
                    end
                end
            end  
            write(file, "end")
        end
    end
end

function create_AllJobsList(subtype,vet_grid,vet_problems)

    PATH = "./testList/"
    cont = 0

    FILE = "testAll_"*subtype*".txt"  


    open(PATH*FILE,"w") do file
        for captype in ["ratio", "rand"]
            for grid in vet_grid 
                c = grid*grid
                vet_numLocations = [ceil(Int,i) for i in [0.5*c,c,1.5*c]]
                if subtype == "2x2" size_S = c/4 elseif subtype == "4x4" size_S = c/16 end
                vet_p = [ceil(Int,i) for i in [size_S,1.25*size_S,1.5*size_S]]
                for prob in vet_problems
                    for loc in vet_numLocations
                        for p_median in vet_p   
                            write(file, "$prob $c $loc $p_median $subtype $captype\n")
                            cont+=1
                        end
                    end
                end
            end
        end
        write(file, "end")
    end
    println(cont)

end


# CPMP 1110 1110 681 commune poste metres
function JOBLIST_PACA()

    PATH = "./testList/"
    cont = 0

    # divVet = ["departament", "canton", "EPCI", "commune"]
    divVet = ["canton", "EPCI"]
    vet_problems = ["CPMP", "CPMP_SC"]

    # typeServ = "poste" #681
    typeServ = "mat" #70
    FILE = "testAll_"*typeServ*".txt"  
    open(PATH*FILE,"w") do file
        for distType in ["metres", "minutes"]
            for prob in vet_problems
                for typeDiv in divVet


                    num_subareas = 0
                    if typeDiv == "commune"  num_subareas = 673 end
                    if typeDiv == "canton"  num_subareas = 31 end
                    if typeDiv == "departement"  num_subareas = 6 end
                    if typeDiv == "EPCI"  num_subareas = 51 end
                    
                    cust, loc = 1110, 1110
                    # for p_median in 681:-50:51
                    for p_median in 70:-4:51
                        if p_median >= num_subareas
                            write(file, "$prob $cust $loc $p_median $typeDiv $typeServ $distType\n")
                            # println("$prob $cust $loc $p_median $typeDiv $typeServ $distType\n")
                        end
                    end
                    write(file, "$prob $cust $loc 51 $typeDiv $typeServ $distType\n")
                end
            end
        end
        write(file, "end")
    end

end



