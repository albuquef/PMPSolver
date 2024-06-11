#@time include("E:\\Optimisation\\chgt_donnees_RF.jl")
#using  DataFrames, CPLEX, JuMP, DelimitedFiles
using  DataFrames, Cbc, JuMP, DelimitedFiles

#PATH_FILES="/home/rosa/Dropbox/MyWork/Research/Geo/Quentin/Model_Xk/toulon/"
#PATH_FILES="/home/figueiredo/Dropbox/MyWork/Research/Geo/Quentin/Optimisation_txt/Model_Xk/FromAnthony/toulon/"
#PATH_FILES="E:\\Optimisation\\"
PATH_FILES="./toulon/"

println("vec de destination.txt")
@time txt_destination=Vector{Int64}(vec(readdlm(PATH_FILES*"toulon_destination.txt")))
#@time txt_destination=Vector{Int64}(vec(readdlm("global_destination.txt")))
println("vec de origin.txt")
@time txt_origin=Vector{Int64}(vec(readdlm(PATH_FILES*"toulon_origin.txt")))
#@time txt_origin=Vector{Int64}(vec(readdlm("global_origin.txt")))
println("vec de tps")
@time txt_tps=Vector{Float64}(vec(readdlm(PATH_FILES*"toulon_tps.txt")))
#@time txt_tps=Vector{Float64}(vec(readdlm("global_tps.txt")))

println("vec de tps.txt * par capacité")
@time txt_orig_x_tps=Vector{Float64}(vec(readdlm(PATH_FILES*"toulon_orig_x_tps.txt")))
#@time txt_orig_x_tps=Vector{Float64}(vec(readdlm("global_orig_x_tps.txt")))

println("Max Dest / Max Orig")
@time max_destination = findmax(txt_destination)
@time max_origin = findmax(txt_origin)

#we are not using now. It is the capacity in (vec de tps.txt * par capacité)
#println("Chargement Origine_OD_avec_Pop.txt : ")
@time d=readdlm(PATH_FILES*"pts_origines_toulon.txt")
#@time d=readdlm("global_pts_origines.txt")

println("Récupération des IDs exactes de DEST & ORIG : ")
@time cpt_nb_dest=DataFrame(A=txt_destination)
@time cpt_nb_origin=DataFrame(B=txt_origin)
@time data_distinct_dest=combine(groupby(cpt_nb_dest, [:A]), nrow => :count)
@time data_distinct_orig=combine(groupby(cpt_nb_origin, [:B]), nrow => :count)

#Trying to reduce the number of variables according to time...
println("DataFrame DEST & ORIG & TPS Complet")
@time df_Dest_Orig_Tps=DataFrame(A=txt_destination, B=txt_origin, C=txt_tps)
println("DataFrame DEST & ORIG & TPS Reduit <= 5min")
@time df_Dest_Orig_Tps_inf5=filter(row -> (row.C <= 15),  df_Dest_Orig_Tps)
@time data_distinct_dest=combine(groupby(df_Dest_Orig_Tps_inf5, [:A]), nrow => :count)
@time data_distinct_orig=combine(groupby(df_Dest_Orig_Tps_inf5, [:B]), nrow => :count)

println("Customers & Locations & Lignes:  ")
customers = Vector{Int64}(vec(data_distinct_orig.B))
locations = Vector{Int64}(vec(data_distinct_dest.A))
lignes = 1:length(txt_orig_x_tps)

p = 5 #5 for Toulon; 100 for paca


#println("Model CPLEX : ")
#@time m = Model(CPLEX.Optimizer)
println("Model Cbc : ")
@time m = Model(Cbc.Optimizer)

println("Variable : ")
  @time @variable(m, x[k in lignes] >=0 )
  @time @variable(m, y[k in locations], Bin)


println("Objective : ")
  @time @objective(m, Min, sum(txt_orig_x_tps[k]*x[k] for k in lignes) )

txt_tps
println("constraint 1 : ")
@time for j in customers
  @constraint(m, sum( x[k] for k in lignes if txt_origin[k]==j) == 1) #chaque origin = un customer
end

println("constraint 2 : ")
  @time @constraint(m, sum( y[i] for i in locations) == p)

println("constraint 3 : ")
 @time for k in lignes
      @constraint(m, x[k] <= y[txt_destination[k]] ) #chaque destination = une location
 end

  println("optimize : ")
  @time optimize!(m)
  println("solve_time : ")
  @time solve_time(m)

  println("Result : ")

list_result=[]

  @time for i in locations
    if JuMP.value.(y[i]) == 1.0
      j=1
      println("Point pour y[i] : ", i)   #Répération des points pour la localisation
      push!(list_result, Int(i))
      push!(list_result, ";")
    end
  end
  @time writedlm(PATH_FILES*"result_optimisation.txt", list_result)
