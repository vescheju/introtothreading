/****************************************************************************
   Justin Vesche
   Computer Project #5
****************************************************************************/

#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <vector>
#include <iomanip>

using namespace std;

// A struct to hold our data
struct Info
{
  vector<string> invInfo; //inventory info container
  vector<string> orderInfo; // order info container
  vector<long> stock; // stock sorted in a container
  bool isEmpty = false; // checks if inventory is completely empty
};

struct Info storageInfo; // global data

 /*---------------------------------------------------------------------------
 Name: ConsumerThread
 Purpose: Uses threading to create a log file and a new inventory
 Receive: void* input for thread
 Return: NULL
 ---------------------------------------------------------------------------*/
void* ConsumerThread(void* input)
{
  ofstream logFile("log");  
  // Loop through the orders
  for(auto i : storageInfo.orderInfo)
  {
		// takes proper strings and places them into
		// a log
    string customerID = i.substr(0,7);
    logFile << customerID << " ";
    string productID = i.substr(8,6);
    logFile << productID << " ";
    string goWithInv;
    bool there = true; // Checks if the item exists
    size_t place = 0; // counter for index
		// if inventory is empty no need to check
    if(!storageInfo.isEmpty)
    {
    for(auto j : storageInfo.invInfo)
    { 
			// Loop through the inventory to find if there
			// exists a product
      if (j.substr(0,6).compare(productID) == 0)
      {
        goWithInv = j;
        break;
      }
      place++;

      if((place) == storageInfo.invInfo.size())
      {
        there = false;
      }
      
    }
    }
    else
    {
      there = false;
    }
   	// if there is the product display its ID
    if (there)
    {
      logFile << left << setw(30)  << goWithInv.substr(19, 30) << " ";
      
    }
    else
    {
      logFile << left << setw(31) << "Unknown Item ";
      cout << "An Unknown Item was inputed." << endl;
    } 
		// How much is being asked for
    string amountOrdered = i.substr(15,5);
    long amountOrderedNum = stol(amountOrdered); // amount ordered
    logFile << setw(5) << right << amountOrderedNum << " ";
   	// again check if we even have the item
    if(there)
    {
			// if we do display the total price of all of it
      string price = goWithInv.substr(7, 5);
      double totalPrice = stod(price); // price of the item
      totalPrice *= amountOrderedNum;
      bool reject = false; // should we reject it?

      logFile << '$';
      logFile << setw(15) << left << setprecision(2) << fixed << totalPrice << \
        " ";
    
      long amountStocked = storageInfo.stock[place]; //amount in stock
			// if the ask is higher than the stock set rejct to true
      if (amountStocked < amountOrderedNum && amountStocked > 0)
      {
        reject = true;
      }
      if(amountStocked >= amountOrderedNum)
      { 
        amountStocked -= amountOrderedNum;
      }
      storageInfo.stock[place] = amountStocked;
      if (amountStocked >= 0 && !reject)
      {
        logFile << setw(8) << left << "Filled"; // if we can proceed with the order
      }
      else
      {
        logFile << setw(8) << left << "Rejected"; // if not reject
      }


    }
    else
    {
			// if we cannot find the item set it to $0.00
      logFile << '$';
      logFile << setw(15) << left << setprecision(2) << fixed << 0.00 << " ";
      logFile << setw(8) << left << "Rejected";
    }


    logFile << '\n';
    

  }
	// create a new invenotory.new
  ofstream newInv("inventory.new");
  size_t counter = 0;
	// check if inv is empty again
  if(!storageInfo.isEmpty)
  {
	// go through the old inventory and update it
  for(auto i : storageInfo.invInfo)
  {
    newInv << i.substr(0,12);
    long stockAmount = storageInfo.stock[counter]; // stock amount
    newInv << " ";
    if (stockAmount < 0)
    {
      stockAmount *= -1;
      newInv << setw(5) << right << stockAmount;
    }
    else
    {
      
      newInv << setw(5) << right << stockAmount;
    }
    
    newInv << " " << i.substr(19,30) << '\n';

    counter++;

  }
  }

  
  logFile.close();
  newInv.close();

  pthread_exit(0);
  return NULL;
}



 /*---------------------------------------------------------------------------
 Name: ProducerThread
 Purpose: Go through the order file and set its vector
 Receive: input for threading
 Return: NULL
 ---------------------------------------------------------------------------*/
void* ProducerThread(void* input)
{
    ifstream file2("orders"); // Check for orders
    string line;
		// if it exists 
    if (file2.is_open()) 
		{
        // fill the lines of the vector
        while(getline(file2, line))
				{		
					storageInfo.orderInfo.push_back(line);
        }		
    }
    else
		{
				// if the file does not exist
        cout << "orders cannot be found" << endl;
        exit(1);
    }
    file2.close();
    pthread_exit(0);
    return NULL;
}



/*---------------------------------------------------------------------------
Name: main
Purpose: uses threading to update the inventory 
return: none
---------------------------------------------------------------------------*/
int main()
{
  pthread_t producerThread; // Threading for the producer
  pthread_t consumerThread; // Threading fot the consumer

  ifstream file1("inventory.old"); // Open this file
  string line; // string for each line

 	// if we can open it
  if(file1.is_open())
  {
    int lineNum = 0; // what line are we on
    while (getline(file1, line))
    {
			// if the file is empty
      if(line == "" && lineNum == 0)
      {
        storageInfo.isEmpty = true;
        break;
      }
      else if(line == "")
      {
        break; // if there is a blank
      }
			// place the according into our vectors
      storageInfo.invInfo.push_back(line);
      string stocked = line.substr(13,5);
      storageInfo.stock.push_back(stol(stocked));

    }
    lineNum++;
  }
  else
  {
			// the file does not exist
      cout << "inventory.old not found" << endl;
      return 1;
  }
	// Now we create and join our threads
  if(pthread_create(&producerThread, NULL, ProducerThread, NULL))
  {
    cout << "Error creating thread" << endl;
    return -1;
  }

  if(pthread_join(producerThread, NULL))
  {
    cout << "Error joining thread" << endl;
    return -1;
  }
  
  
 if(pthread_create(&consumerThread, NULL, ConsumerThread, NULL))
  {
    cout << "Error creating thread" << endl;
    return -1;
  }

  if(pthread_join(consumerThread, NULL))
  {
    cout << "Error joining thread" << endl;
    return -1;
  }

  file1.close();
}

