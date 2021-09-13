import java.util.Scanner;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.InputMismatchException;
import java.util.Random;

class ClientThread extends Thread{
    // IO stuff
    Socket clientSocket;
    DataInputStream input;
    DataOutputStream output;

    // Thread task metadata
    int option; // option to execute on the servers
    int key; // the integer key to look for if searching
    int[] data; // the local copy of data for each thread
    int num; // the number of the thread in the thread array

    // Shared static variables for holding thread results
    // Doing this since threads cannot return data
    // But it would make more sense to impelemnt Callables instead of Threads
    static int result;
    static int found_by;
    static int num_threads;
    static int[][] results;
    
    // Client Thread Constuctor   
    ClientThread(String hostName, int portNumber, int option, int[] data, int key, int num){
       try{
          this.clientSocket = new Socket(hostName, portNumber);
          this.input = new DataInputStream(clientSocket.getInputStream());
          this.output = new DataOutputStream(clientSocket.getOutputStream());
          this.option = option; 
          this.data = data; 
          this.key = key; 
          this.num = num; 
       }catch (UnknownHostException e){
          System.err.println("Dont know about host " + hostName);
          System.exit(1);
       }catch (IOException e){
          System.err.println("Couldn't get I/O for the connection to " + hostName);
          System.exit(1);
       }
    }
    
    //The code that the threads execute when started
    public void run(){

      // Send server the data and metadata
      try{
         this.output.writeInt(option); // write option to server
         if (option == 2) this.output.writeInt(this.key); // write key if searching
         this.output.writeInt(this.data.length); // write length of data to server
         for (int i : data) this.output.writeInt(i); // write data to server
      } catch (IOException ioe){
         System.err.println("I/O error writing to the server.");
         System.exit(1);
      }

      // Collect the server's response
      int res = -1;
      try {
         if (option == 1 || option == 2) res = input.readInt();
         else for (int i = 0; i < data.length; i++) data[i] = input.readInt();
      } catch (IOException ioe){
         System.err.println("I/O error reading the server's response.");
         System.exit(1);
      }

      // Process result
      if (option == 1){ // let the first thread update the result
         if (num == 0) result = res; 
      } else if (option == 2) { // update result if thread found key
         if (res != -1) { found_by = num; result = res; }
      } else if (option == 3){ // store the sorted array into a static 2D array
         results[num] = data; 
      }
         
       // Close the connection
       try{
          this.input.close();
          this.output.close();
          this.clientSocket.close();
       }catch (Exception e){
          System.err.println("Error closing with the connections");
          System.exit(1);
       }
    }
 }

public class UranusClient{
   public static void main(String[] args) throws IOException{

      //check that necessary info is provided
      int N = 0;
      if (args.length != 1){
         System.out.println("Must use format: 'java Client (array_size)'\nExiting Program.....");
         System.exit(1);
      }
      try {
         N = Integer.parseInt(args[0]);
      } catch (InputMismatchException ime) {
         System.out.println("Please enter an integer!");
         System.exit(1);
      }

      // Some static variables and info
      int menuOp;
      int port = 8888;
      String hosts[] = new String[] {
                     "compute-0-0.local","compute-0-1.local",
                     "compute-0-2.local","compute-0-3.local",
                     "compute-0-4.local","compute-0-5.local",
                     "compute-0-6.local","compute-0-7.local",
                     "compute-0-8.local","compute-0-9.local",
                     "compute-0-10.local","compute-0-11.local",
                     "compute-0-12.local"};
      ClientThread[] threads = new ClientThread[hosts.length]; 
      Scanner sc = new Scanner(System.in);
      boolean parallel = true; // flag to conduct operations in parallel

      
      // Generate the random array
      System.out.printf("Generating the random array with %d elements...\n",N);
      int[] data = new int[N];
      data = (new Random()).ints((long) N, 0, 10000).toArray();

      // the main client program loop
      while(true){

         // Print the menu
         System.out.println("\nEnter one of the following commands:");
         System.out.println("0 - Print the array");
         System.out.println("1 - Return the first element");
         System.out.println("2 - Linear Search");
         System.out.println("3 - Bubble sort");
         System.out.println("4 - Matrix multiply");
         System.out.println("5 - Toggle Parallelism");
         System.out.println("6 - Exit");

         // Get the menu choice
         while (true){
            try{
               System.out.print("Command: ");
               menuOp = Integer.parseInt(sc.next());
            }catch(NumberFormatException nfe){
               System.out.println("Please enter a number between 0 and 6");
               continue;
            }
            if (menuOp < 0 || menuOp > 6)
               System.out.println("Please enter a number between 0 and 6");
            else break;
         }

         // Process the easy options (printing array, exiting, and toggling parallelsim)
         if (menuOp == 0){for (int i:data)System.out.printf("%d ",i);continue;}
         else if (menuOp == 6){System.out.println("Exiting program");System.exit(1);} 
         else if (menuOp == 5) {parallel = !parallel; continue;}
         else if (menuOp == 3) {data = (new Random()).ints((long) N, 0, 10000).toArray();} //rerandomize data
         else if (menuOp == 4) {System.out.println("Not implemented for Uranus! :(");continue;}

         // Prompt for key if option 2 selected
         int key = -1;
         if (menuOp == 2){
            System.out.println("Enter the integer to search for: ");
            key = Integer.parseInt(sc.next());
         } // handle invalid input later
         System.out.println("Executing command: " + menuOp);

         // Split up the array and create the threads
         int num_hosts = parallel ? hosts.length : 1; // use first node if not parallel
         int start, stop, interval = N / hosts.length;
         int[] slice;
         for (int i = 0; i < num_hosts; i++){
            start = i * interval;
            stop = i == num_hosts - 1 ? N : start + interval;
            //System.out.printf("Thread %d(Start: %d,Stop: %d)",i,start,stop);
            slice = Arrays.copyOfRange(data, start, stop);
            threads[i] = new ClientThread(hosts[i], port, menuOp, slice, key, i);
         }

         // Initialize static shared variables, start timer, and execute threads
         ClientThread.num_threads = num_hosts;
         ClientThread.results = new int[num_hosts][];
         ClientThread.result = -1;
         ClientThread.found_by = -1;
         long startTime = System.currentTimeMillis();
         for (int j = 0;j < num_hosts;j++) threads[j].start();
        
         //join after all threads have started so that the program waits for all of them to finish
         for (int k = 0; k < num_hosts; k++){
            try{
               threads[k].join();
            }catch (InterruptedException ie){
               System.out.println(ie.getMessage());
            }
         }

         // Final post processing and presentation of results to client
         switch (menuOp){
            case 1:
               int first = ClientThread.result;
               System.out.printf("The first item in the array is %d",first);
               break;
            case 2:
               int found_by = ClientThread.found_by;
               int index = ClientThread.result;
               int adj_index = found_by * interval + index;
               if (index != -1) {
                  System.out.printf("The key was found at index %d by thread %d\n",index,found_by);
                  System.out.printf("So in the whole array it is at index %d",adj_index);
               } else System.out.println("The key was not found in the array.");
               break;
            case 3:
               data = merge(ClientThread.results, num_hosts, N);
               System.out.println("Sorted Data: ");
               for (int i : data) System.out.printf("%d ",i);
               break;
            case 4:
               break;
         }

         // Report time elapsed from thread execution to end of processing of results
         long elapsedTime = System.currentTimeMillis() - startTime;
         System.out.printf("\nComputation took %d ms\n", elapsedTime);
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
}