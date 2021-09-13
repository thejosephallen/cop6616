import java.util.Scanner;
import java.io.IOException;
import java.util.Arrays;
import java.util.InputMismatchException;
import java.util.Random;

class AtlasThread extends Thread{

    // Thread task metadata
    int option; // option to execute on the servers
    int key; // the integer key to look for if searching
    int[] data; // the local copy of data for each thread
    int[][] matrix; // local copy of matrix
    int num; // the number of the thread in the thread array
    
    // Time attributes
    // static ArrayList<Long> times = new ArrayList<Long>();
    // long request_time = 0;

    // Shared static variables for holding thread results
    // Doing this since threads cannot return data
    // But it would make more sense to impelemnt Callables instead of Threads
    static int result;
    static int found_by;
    static int num_threads;
    static int[][] results;
    
    // Atlas Thread Constuctor   
    AtlasThread(int option, int[] data, int[][] matrix, int key, int num){
          this.option = option; 
          this.data = data; 
          this.matrix = matrix;
          this.key = key; 
          this.num = num; 
    }
    
    //The code that the threads execute when started
    public void run(){

        // perform tasks here

        switch (option){
            case 1: // O(1)
                if (num == 0) result = data[0]; // let first thread set result
                break;
            case 2: // O(n)
                // Linear search for key
                int index = -1;
                for (int i = 0; i < data.length; i++){
                    if (data[i] == key){
                        index = i;
                        break;
                    }
                }
                // let thread update result if it found the key
                if (index != -1){
                    synchronized(this){
                        found_by = num;
                        result = index;
                    } // thread safe in case multiple threads find key
                } 
                break;
            case 3: // O(n^2)
                for (int i = 0; i < data.length; i++){
                    for (int j = i + 1; j < data.length; j++){
                        if (data[i] > data[j]){
                            int temp = data[i];
                            data[i] = data[j];
                            data[j] = temp;
                        }
                    }
                }
                results[num] = data;
                break;
            case 4:
                // for each row for this thread
                for (int i : data) {
                    // for every column of the second matrix
                    for (int j = 0; j < matrix.length; j++) {
                        // for each row of the second matrix
                        for (int k = 0; k < matrix.length; k++) {
                            //System.out.printf("Thread=%d,i=%d,j=%d,k=%d\n",num,i,j,k);
                            results[i][j] = results[i][j] + matrix[i][k] * matrix[k][j];
                        }
                    }
                }
                break;
        }
    }
}
public class Atlas{
   public static void main(String[] args) throws IOException{

        int num_threads = 1;

      //check that necessary info is provided
      int N = 0;
      if (args.length != 1){
         System.out.println("Must use format: 'java Atlas (array_size)'\nExiting Program.....");
         System.exit(1);
      }
      try {
         N = Integer.parseInt(args[0]);
      } catch (InputMismatchException ime) {
         System.out.println("Please enter an integer!");
         System.exit(1);
      }

      // static port and hostname info
      int menuOp;
      int key = -1;
      AtlasThread[] threads = new AtlasThread[num_threads];
      Scanner sc = new Scanner(System.in);
         
      // Generate the random array
      System.out.printf("Generating the random array with %d elements...\n",N);
      int[] data = new int[N];
      int[][] matrix = new int[N][];
      data = (new Random()).ints((long) N, 0, 10000).toArray();

        boolean matmul = false;
        boolean printResults = false;
        while(true){

         // Print the menu
         System.out.println("\n\nEnter one of the following commands:");
         System.out.println("0 - Print the array");
         System.out.println("1 - Return the first element");
         System.out.println("2 - Linear Search");
         System.out.println("3 - Bubble sort");
         System.out.println("4 - Matrix multiply");
         System.out.printf("5 - Modify thread count (currently %d)\n",num_threads);
         System.out.printf("6 - Toggle printing (currently %b)\n",printResults);
         System.out.println("7 - Exit");

         // Get the menu choice
         while (true){
            try{
               System.out.print("Command: "); menuOp = Integer.parseInt(sc.next());
            }catch(NumberFormatException nfe){
               System.out.println("Please enter a number between 0 and 7"); continue;
            }
            if (menuOp < 0 || menuOp > 7)
               System.out.println("Please enter a number between 0 and 7");
            else break;
         }

         // Preprocessing of menu selection
         System.out.println("Executing command: " + menuOp);
         switch (menuOp){
            case 0: // print array
                for (int i:data)System.out.printf("%d ",i); continue;
            case 6: // toggle printing
                printResults = !printResults; continue;
            case 7: // exit
                System.out.println("Exiting program"); System.exit(1);
            case 3: //rerandomize data
                data = (new Random()).ints((long) N, 0, 10000).toArray();
                break; 
            case 5: // modify thread count 
                System.out.print("Enter the new number of threads: ");
                num_threads = Integer.parseInt(sc.next()); // handle invalids later
                threads = new AtlasThread[num_threads]; 
                continue;
            case 2: // prompt for key to search for
                System.out.print("Enter the integer to search for: ");
                key = Integer.parseInt(sc.next()); // handle invalids later
                break;
            case 4: // generate a random square matrix of size N xN
                for (int i = 0; i < N; i++)
                    matrix[i] = (new Random()).ints((long) N, 0, 10).toArray();
                matmul = true;
                break;
        }
  
         // Split up the array and create the threads
         int start, stop, interval = N / num_threads;
         int[] slice;
         for (int i = 0; i < num_threads; i++){
            start = i * interval;
            stop = i == num_threads - 1 ? N : start + interval;
            //System.out.printf("Thread %d(Start: %d,Stop: %d)",i,start,stop);
            if (matmul) {
                slice = range(start, stop); // use array of row indices
                threads[i] = new AtlasThread(menuOp, slice, matrix, key, i);
            } else {
                slice = Arrays.copyOfRange(data, start, stop); // use slice of data
                threads[i] = new AtlasThread(menuOp, slice, null, key, i);
            }
         }

         // Initialize static shared variables, start timer, and execute threads
         AtlasThread.num_threads = num_threads;
         AtlasThread.result = -1;
         AtlasThread.found_by = -1;
         if (matmul) AtlasThread.results = new int[N][N]; // N x N if matmul
         else AtlasThread.results = new int[num_threads][]; // else, num_threads x slice         
         long startTime = System.currentTimeMillis();
         for (int j = 0;j < num_threads;j++) threads[j].start();
        
         //join after all threads have started so that the program waits for all of them to finish
         for (int k = 0; k < num_threads; k++){
            try{
               threads[k].join();
            }catch (InterruptedException ie){
               System.out.println(ie.getMessage());
            }
         }

         // final post processing if sorting
         if (menuOp == 3) data = merge(AtlasThread.results, num_threads, N);

         // Time is up
         long elapsedTime = System.currentTimeMillis() - startTime;
         System.out.printf("\nComputation took %d ms\n", elapsedTime);

         // Presentation of results
         switch (menuOp){
            case 1:
               int first = AtlasThread.result;
               if (printResults) System.out.printf("The first item in the array is %d",first);
               break;
            case 2:
                int found_by = AtlasThread.found_by;
                int index = AtlasThread.result;
                int adj_index = found_by * interval + index;
                if (printResults) {
                    if (index != -1) {
                        System.out.printf("The key was found at index %d by thread %d\n",index,found_by);
                        System.out.printf("So in the whole array it is at index %d",adj_index);
                    } else System.out.println("The key was not found in the array.");
                }
               break;
            case 3:
                if (printResults) {
                    System.out.println("Sorted Data: ");
                    for (int i : data) System.out.printf("%d ",i);
                }
               break;
            case 4:
                if (printResults) {
                    System.out.println("The square matrix to be multiplied by itself: ");
                    for (int i = 0; i < N; i++) {
                        for (int j = 0; j < N; j++) 
                            System.out.printf("%d ",matrix[i][j]);
                        System.out.println();
                    }
                    System.out.println("The result of matrix multiplication: ");
                    for (int i = 0; i < N; i++) {
                        for (int j = 0; j < N; j++) 
                            System.out.printf("%6d ",AtlasThread.results[i][j]);
                        System.out.println();
                    }
                }
                matmul = false;
                break;
         }
         // Report time elapsed from thread execution to end of processing of results

    }
   }//end main

   /**
    * A very naive implementation of merge (due to array copy to update rows).
    * Definitely implement with a min heap later on.
    * @param lists - the array of arrays which are not necessarily of the same length
    * @param rows - the number of threads and rows in lists
    * @param N - the number of items to merge into a single array
    * @return returns the merged and sorted array
    */
   public static int[] merge(int[][] lists, int rows, int N){
      int min_int;
      int min_row = 0;
      int[] sorted = new int[N];
      for (int i = 0; i < N; i++){
         min_int = Integer.MAX_VALUE;
         for (int j = 0; j < rows; j++){
            if (lists[j].length > 0 && lists[j][0] < min_int){
               min_int = lists[j][0];
               min_row = j;
            } 
         }
         sorted[i] = min_int; // add min to result and adjust the array it came from
         int[] new_row = new int[lists[min_row].length - 1];
         for (int k = 1; k < lists[min_row].length; k++) new_row[k-1] = lists[min_row][k];
         lists[min_row] = new_row;
         //System.arraycopy(lists[min_row], 1, lists[min_row], 0, lists[min_row].length - 1);
      }
      return sorted;
   } 

    public static int[] range(int start, int stop){
        //System.out.printf("Start: %d ---- Stop: %d\n",start,stop);
        int[] result = new int[stop - start];
        for (int i = start; i < stop; i++) result[i - start] = i;
        //for (int i : result) System.out.printf("%d ",i); 
        return result;
   }
}