import java.net.*;
import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.DateFormat;
import java.lang.Process;
import java.lang.Runtime;

class ServerThread implements Runnable{
   //IO and attributes for each thread
   Socket client;
   String response;
   String clientAddress;
   DataInputStream input; // for reading and writing ints
   DataOutputStream output;
   
   // Server Thread Constructor
   ServerThread(Socket client){
      try {
         this.client = client;
         this.input = new DataInputStream(client.getInputStream());
         this.output = new DataOutputStream(client.getOutputStream());
         this.clientAddress = client.getInetAddress().toString();
         System.out.println("Connection successful with user: " + this.clientAddress);
         (new Thread(this)).start();
      } catch (IOException e) {
         System.out.println("Exception caught when trying to set up IO.");
         System.out.println(e.getMessage());
      }
   }
   
   // What the threads do
   public void run(){
      try{         

         // Process request header and read in the data
         int request = input.readInt(); // get the client's request
         System.out.printf("Client requested option %d\n",request);
         int key = -1; // get the key if request is to search
         if (request == 2) key = input.readInt(); 
         int len = input.readInt(); // get the size of the data sent
         int[] data = new int[len]; // initialize an array to hold it
         for(int i = 0; i < len; i++) data[i] = input.readInt();

         switch (request){
            case 1: // O(1)
               System.out.println("Received request to return first!");
               output.writeInt(data[0]); //return the first element
               break;
            case 2: // O(n)
               System.out.println("Received request to linear search!");

               // Linear search for key
               int index = -1;
               for (int i = 0; i < len; i++){
                  if (data[i] == key){
                     index = i;
                     System.out.printf("Found key at index %d",i);
                     break;
                  }
               }

               // return the index (-1 if not found)
               if (index == -1) System.out.println("Could not find the client's key.");
               else System.out.printf("Found Client's key at index %d",index);
               output.writeInt(index);

               break;
            case 3: // O(n^2)
               System.out.println("Received request to bubble sort!");

               // bubble sort the data
               for (int i = 0; i < len; i++){
                  for (int j = i + 1; j < len; j++){
                     if (data[i] > data[j]){
                        int temp = data[i];
                        data[i] = data[j];
                        data[j] = temp;
                     }
                  }
               }

               // Printd data and send output back
               System.out.println("Sorted data: ");
               for (int i : data) { System.out.printf("%d ",i); output.writeInt(i); }

               break;
            case 4: // O(n^3)
               System.out.println("Received request to matrix multiply!");
               break;
         }        

         // Close I/O and socket
         System.out.println("\nClosing connection with user: " + this.clientAddress);   
         this.input.close();
         this.output.close();
         this.client.close();
      }catch (IOException e){
         System.err.println("I/O error with the connection");
         System.exit(1);
      }   
   }
}

public class UranusServer {
   public static void main(String[] args) throws IOException{
      // if (args.length != 1){
      //    System.err.println("Usage: java Server <port number>");
      //    System.exit(1);
      // }
      int portNumber = 8888;//Integer.parseInt(args[0]); //hardcoded for now
      try{
         ServerSocket server = new ServerSocket(portNumber, 100); // 100 is connection queue size
         System.out.println("(Type CTRL+C to end the server program)");
         while(true) new ServerThread(server.accept()); // Server loop
      } catch(IOException e) {
         System.out.println("Exception caught when trying to listen on port " + portNumber);
         System.out.println(e.getMessage());
      }
   }
}