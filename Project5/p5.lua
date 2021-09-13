-- import necessary modules
socket = require "socket" -- for execution timers
lanes = require "lanes".configure() -- import and configure the lanes module
lanes.register("io",io) -- register the io module for use within a lane
os = require "os" 
math.randomseed(os.time())  -- comment out for static randomness
output = false -- flag for whether to output results

--------------------------- PARALLEL FUNCTIONS --------------------------------
-- return the first item in an array
local function return_first(array, key)
    return array[1]
end

-- linear search through an array
local function linear_search(array, key)
    for i = 1, #array, 1 do
        if array[i] == key then
            return i
        end
    end
    return 0
end

-- bubble sort an array of numbers from high to low
local function bubble_sort(array, key)
    for i = 1, #array - 1, 1 do
        for j = i + 1, #array, 1 do
            if array[j] > array[i] then
                array[j], array[i] = array[i], array[j] -- python swapping, cool!
            end
        end
    end
    return array
end

-- parallel matrix multiplication of a flattened 1D square matrix with itself
local function matrix_mult(flat_matrix, metadata)
    s = metadata[1] -- number of rows per lane (except maybe the last lane)
    N = metadata[2] -- matrix is N x N
    rank = metadata[3] -- lane id

    -- the num of rows for this lane to process (adjust if the last lane has fewer)
    if (rank + 1) * s < N then num_rows = s
    else num_rows = N - (rank * s) end

    -- matrix multiply the assigned rows with all columns
    result = {} 
    i = rank * s * N
    j = 0
    while i < (rank * s * N) + (num_rows * N) do -- indices may be screwed thx to 1-indexing -_-
        for k = 1, k < N, 1 do
            for l = 1, l < N, 1 do
                result[j * N + k] = result[j * N + k] + flat_matrix[i + l] * flat_matrix[l * N + k]
            end
        end
        i = i + N
        j = j + 1
    end

    return result
end

--------------------------------- UTILS ---------------------------------------
-- generate an array of random integers in [1,1000]
function random_array(size)
    array = {}
    for i = 1, size, 1 do
        array[i] = math.random(1000)
    end
    return array
end

-- generate a square matrix of random integers in [1,1000]
function random_matrix(size)
    matrix = {}
    for i = 1, size, 1 do
        matrix[i] = random_array(size)
    end
    return matrix
end

-- print an array
function print_array(array)
    for i = 1, #array, 1 do
        io.write(array[i], ' ')
    end
    io.write('\n')
end

-- print a matrix
function print_matrix(matrix)
    for i = 1, #matrix, 1 do
        print_array(matrix[i])
    end
end

-- merge an array of sorted arrays (high to low) into a single sorted array (low to high)
function merge(arrays)
    new = {}
    new_len = (#arrays - 1) * #arrays[1] + #arrays[#arrays]
    for i = 1, new_len, 1 do -- iterate over new array
        min = math.huge -- largest number
        min_index = 0
        for j = 1, #arrays, 1 do -- scan end of each subarray
            cur = arrays[j][#(arrays[j])]
            if cur ~= nil and cur < min then
                min = cur
                min_index = j
            end
        end
        new[i] = table.remove(arrays[min_index]) -- O(1) pop
    end
    return new
end

------------------------------------ MAIN -------------------------------------
-- get the command line args needed
option = tonumber(arg[1])  -- the function to execute
N = tonumber(arg[2])  -- the number of items in the generated array
T = tonumber(arg[3])  -- the number of threads to use

-- get the matrix or array and print it
if option == 3 then 
    matrix = random_matrix(N)
    if output then
        print("\nRandom matrix: ")
        print_matrix(matrix)
    end
    print("Matrix multiplication is not fully implemented yet :(")
    os.exit()
else
    array = random_array(N)
    if output then
        print("\nRandom array: ")
        print_array(array)
    end
end

-- divide the array into a 2D table of tables for each thread to process
slices = {}
s = N / T -- array slice size per thread (not exact)
for i = 1, T, 1 do
    slices[i] = {}
    for j = 1, s, 1 do
        index = (i - 1) * s + j -- index into original array
        if index > N then break end -- break if reached end of data (uneven slices)
        slices[i][j] = array[index]
    end
end

-- select the right function and initialize a random key if searching
local key = nil
local selected_function = nil
if option == 0 then
    selected_function = return_first
elseif option == 1 then
    selected_function = linear_search
    key = array[math.random(#array)]
    if output then
        print("The key is: ", key)
    end
else
    selected_function = bubble_sort
end

-- wrap the parallel functions for sync
local sync_linda = lanes.linda() -- linda object for lane syncing
fun = lanes.gen(
    function(slice, key) -- key param unused except in search
        result = selected_function(slice, key)
        sync_linda:send("done", true) 
        return result
    end
)

-- start the timer
start_time = socket.gettime()

-- start each lane and store it in a lane table
lane_table = {}
for i=1, T, 1 do
    lane_table[i] = fun(slices[i], key)
end

-- wait for all lanes to write to linda's done field (sync)
sync_linda:receive(nil, sync_linda.batched, "done", #lane_table) 

-- collect results from each lane
for i=1, T, 1 do
    slices[i] = lane_table[i][1]  -- index into the lane table to access returned result
    if option == 0 then
        if output and i == 1 then
            print(string.format("\nThe first item in the array is %d", slices[i]))
            break
        end
    elseif option == 1 then
        if output and slices[i] ~= 0 then
            print(string.format("\nLane %d found %d at index %d", i, key, (i-1) * s + slices[i]))
        end
    else
        if output then
            print(string.format("\nLane %d gives: ", i))
            print_array(slices[i])
        end
    end
end

-- some final processing for bubble sort
if option == 2 then
    -- merge the sorted subarrays
    array = merge(slices)
    elapsed_time = socket.gettime() - start_time -- stop the timer

    -- print the final sorted array
    if output then
        print("\nSorted array: ")
        print_array(array)
    end
else
    elapsed_time = socket.gettime() - start_time -- just stop the timer    
end

-- print the final execution time
print(string.format("\nExecution time: %.4f ms", elapsed_time * 1000))
