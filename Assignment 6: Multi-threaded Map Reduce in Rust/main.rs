use std::env; // to get arugments passed to the program
use std::thread;

/*
* Print the number of partitions and the size of each partition
* @param vs A vector of vectors
*/
fn print_partition_info(vs: &Vec<Vec<usize>>){
    println!("Number of partitions = {}", vs.len());
    for i in 0..vs.len(){
        println!("\tsize of partition {} = {}", i, vs[i].len());
    }
}

/*
* Create a vector with integers from 0 to num_elements -1
* @param num_elements How many integers to generate
* @return A vector with integers from 0 to (num_elements - 1)
*/
fn generate_data(num_elements: usize) -> Vec<usize>{
    let mut v : Vec<usize> = Vec::new();
    for i in 0..num_elements {
        v.push(i);
    }
    return v;
}

/*
* Partition the data in the vector v into 2 vectors
* @param v Vector of integers
* @return A vector that contains 2 vectors of integers

*/
fn partition_data_in_two(v: &Vec<usize>) -> Vec<Vec<usize>>{
    let partition_size = v.len() / 2;
    // Create a vector that will contain vectors of integers
    let mut xs: Vec<Vec<usize>> = Vec::new();

    // Create the first vector of integers
    let mut x1 : Vec<usize> = Vec::new();
    // Add the first half of the integers in the input vector to x1
    for i in 0..partition_size{
        x1.push(v[i]);
    }
    // Add x1 to the vector that will be returned by this function
    xs.push(x1);

    // Create the second vector of integers
    let mut x2 : Vec<usize> = Vec::new();
    // Add the second half of the integers in the input vector to x2
    for i in partition_size..v.len(){
        x2.push(v[i]);
    }
    // Add x2 to the vector that will be returned by this function
    xs.push(x2);
    // Return the result vector
    xs
}

/*
* Sum up the all the integers in the given vector
* @param v Vector of integers
* @return Sum of integers in v
* Note: this function has the same code as the reduce_data function.
*       But don't change the code of map_data or reduce_data.
*/
fn map_data(v: &Vec<usize>) -> usize{
    let mut sum = 0;
    for i in v{
        sum += i;
    }
    sum
}

/*
* Sum up the all the integers in the given vector
* @param v Vector of integers
* @return Sum of integers in v
*/
fn reduce_data(v: &Vec<usize>) -> usize{
    let mut sum = 0;
    for i in v{
        sum += i;
    }
    sum
}

/*
* A single threaded map-reduce program
*/
fn main() {

    // Use std::env to get arguments passed to the program
    let args: Vec<String> = env::args().collect();
    if args.len() != 3 {
        println!("ERROR: Usage {} num_partitions num_elements", args[0]);
        return;
    }
    let num_partitions : usize = args[1].parse().unwrap();
    let num_elements : usize = args[2].parse().unwrap();
    if num_partitions < 1{
      println!("ERROR: num_partitions must be at least 1");
        return;
    }
    if num_elements < num_partitions{
        println!("ERROR: num_elements cannot be smaller than num_partitions");
        return;
    }

    // Generate data.
    let v = generate_data(num_elements);

    // PARTITION STEP: partition the data into 2 partitions
    let xs = partition_data_in_two(&v);

    // Print info about the partitions
    print_partition_info(&xs);

    let mut intermediate_sums : Vec<usize> = Vec::new();

    // MAP STEP: Process each partition

    // clone the partition into two seperate variables
    let xs_clone_one = xs.clone();
    let xs_clone_two = xs.clone();

    // -------------------- Timur Code Changes Begins ------------------------------

    // spawn a thread for each partitioned cloned with the correct vector index
    let t1 = thread::spawn(move || map_data(&xs_clone_one[0]));
    let t2 = thread::spawn(move || map_data(&xs_clone_two[1]));

    // join the partitions and push each to intermediate sums
    intermediate_sums.push(t1.join().unwrap());
    intermediate_sums.push(t2.join().unwrap());

    // ---------------------- Timur Code Changes Ends ------------------------------

    // Print the vector with the intermediate sums
    println!("Intermediate sums = {:?}", intermediate_sums);

    // REDUCE STEP: Process the intermediate result to produce the final result
    let sum = reduce_data(&intermediate_sums);
    println!("Sum = {}", sum);
    
    // -------------------- Timur Code Changes Begins ------------------------------

    // 1. Calls partition_data to partition the data into equal partitions
    let xs_two = partition_data(num_partitions, &v);

    // 2. Calls print_partition_info to print info on the partitions that have been created
    print_partition_info(&xs_two);

    // 3. Creates one thread per partition and uses each thread to concurrently process one partition
    // CITE: https://doc.rust-lang.org/rust-by-example/std_misc/threads.html
    let mut thread_spawns = vec![];

    // create threads like in part one but use for loop and store the threads in a vector
    for i in 0..xs_two.len(){
        let temp_clone = xs_two.clone();
        let temp_thread = thread::spawn(move || map_data(&temp_clone[i]));
        thread_spawns.push(temp_thread);
    }

    // 4. Collects the intermediate sums from all the threads
    // declare a vector for sums just like in part part one
    let mut intermediate_sums_two : Vec<usize> = Vec::new();

    // join the partitions and push each to intermediate sums like in but using a for loop
    for cur_thread in thread_spawns{
        intermediate_sums_two.push(cur_thread.join().unwrap());
    }

    // 5. Prints information about the intermediate sums
    // Print the vector with the intermediate sums
    println!("Intermediate sums = {:?}", intermediate_sums_two);

    // 6. Prints the final sum computed by reduce_data
    // REDUCE STEP: Process the intermediate result to produce the final result
    let sum = reduce_data(&intermediate_sums);
    println!("Sum = {}", sum);

    // ---------------------- Timur Code Changes Ends ------------------------------
    
}

// -------------------- Timur Code Changes Begins ------------------------------

fn partition_data(num_partitions: usize, v: &Vec<usize>) -> Vec<Vec<usize>>{

    // Create a vector that will contain vectors of integers
    let mut xs: Vec<Vec<usize>> = Vec::new();

    // get the partition size and the modulus remainder if not evenly divisible and the current index of v
    let partition_size = v.len() / num_partitions;
    let mut partition_remainder = v.len() % num_partitions;
    let mut index = 0;

    // for loop that will iteration and produce vectors by number of partitions
    for _i in 0..num_partitions{
        // if there is no remainder than proceed
        if partition_remainder != 0{
            // create a temp vertor to store the data between 
            let mut xsr : Vec<usize> = Vec::new();
            // for loop through the current indices of v and push to temp vector, increment index
            for _x in 0..partition_size+1{
                xsr.push(v[index]);
                index += 1;
            }
            // push temp vector to xs and decrement partition_remainder.
            xs.push(xsr);
            partition_remainder -= 1;
        }
        // else do the same except with other the partition_remainder
        else{
            let mut xsn : Vec<usize> = Vec::new();
            for _x in 0..partition_size{
                xsn.push(v[index]);
                index += 1;
            }
            xs.push(xsn);
        }
    }

    // return xs
    xs
}

// ---------------------- Timur Code Changes Ends ------------------------------