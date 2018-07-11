/*
 AUTHOR : BRENDAN OSBERG: July 2016  
 You are free to copy and redistribute this script, provided you cite and credit 

 projects proportional seating arrangements for the MMM model,
 based on raw FPP input.

*/

// Include various dependencies 
#include <fstream>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <queue>

using namespace std;


// Define class object "quotient"  (Q objects described in paper)

class quotient{   
  public:
  quotient();
  double  value;     // generally, # of votes divided by jval (below)
  int     jval;      // array of values: 1,2,3,... etc. 
  bool    assigned;  // has this quotient been attributed to a seat yet?
  string party_att;  // Three character string denoting party
  };

quotient::quotient()
  {
  value     = 0.0;
  jval      = 0;
  assigned  = false;
  party_att ='\0';
  }

//----------------------------------------------


// Define class object "party" 
class party{

  public:
  party();
  string name;
  int votes;
  double vote_share;
  int seats_initial;  // # seats assigned directly from constituencies
  int seats_assigned; // # seats currently assigned at a given moment 
                      // within this algorithm.

  bool most_over_represented;
  double * party_quotient_list;
  };

party::party()
  {
  name                  = '\0';
  votes                 = 0;
  seats_initial         = seats_assigned = 0; 
  vote_share            = 0.0;
  most_over_represented = false;
  }



bool should_terminate ( quotient * Total_quotient_list, party * all_parties, int qi, int Seats_total_init, int & total_seats_assigned, int Seats_max_cutoff, int Num_parties, int total_votes, bool count_NotA );

//******************************************************************

int main(int argc, char *argv[])
{

// Declare and initialize variables 
string FILE_in;  // input file path taken as command-line argument
string FILE_out; // output specified by user at runtime
string prompt;

ifstream datin;
ofstream datout;

int Seats_total_init=0;
int Seats_assigned_final=0;
int Seats_max_cutoff;

int total_votes=0;
int i=0, j=0;

for( i = 1; i < argc; i++ )
  { FILE_in += argv[i]; }

// I/O filestream
cout << "\n Input file name is read as: " << FILE_in << endl;
cout << "\n Enter output FILE name: ";
cin >> FILE_out;

bool count_NotA;
cout << "\n\n  Should NotA (None of the Above) votes be counted? (enter [y/n]): " << endl ;
cin  >> prompt;

if ( prompt == "Y" || prompt == "y")
  { count_NotA = true; }
else if ( prompt == "N" || prompt == "n")
  { count_NotA = false; }
else
  { 
  cout << "Input not understood. Exiting." << endl;
  exit(1);
  }



// Populate party list and read in member data from input file:

int Num_parties=6;    // "Other" is treated as a party (although awarded no seats)
party all_parties[Num_parties];

datin.open(FILE_in.c_str());
datout.open(FILE_out.c_str());


if( datin.fail() )
  {
  cout << "\n ERROR, can't find input file. exiting \n";
  exit(1);
  }
else
  {//----succeeded reading in file
  for(i=0;i<Num_parties;i++)
    {
    datin >>  all_parties[i].name; 
    datin >>  all_parties[i].seats_initial;  
    datin >>  all_parties[i].votes;

    all_parties[i].seats_assigned = all_parties[i].seats_initial;

    Seats_total_init += all_parties[i].seats_initial;
    total_votes      += all_parties[i].votes;
    }

  }
Seats_max_cutoff = 2*Seats_total_init;
 
// populate each party's list of quotients 
quotient Total_quotient_list[Num_parties*2*Seats_total_init];

for(i=0;i<Num_parties;i++)
  {
  all_parties[i].vote_share          = double(all_parties[i].votes)/double(total_votes); 
  all_parties[i].party_quotient_list = new double[2*Seats_total_init];
  
  for(j=0;j<2*Seats_total_init;j++)
    {
    all_parties[i].party_quotient_list[j] = double(all_parties[i].votes)/(double(j+1));

    //   location:  -->|_______________________|<--  is just to assign a slot in memory. 
    Total_quotient_list[i*(2*Seats_total_init)+j].party_att = all_parties[i].name;
    Total_quotient_list[i*(2*Seats_total_init)+j].jval      = j;
    Total_quotient_list[i*(2*Seats_total_init)+j].value     = double(all_parties[i].votes)/(double(j+1));

    if ( j >= all_parties[i].seats_initial )
       {
       Total_quotient_list[i*(2*Seats_total_init)+j].assigned  = false;
       }
    else if ( j < all_parties[i].seats_initial )
       {
       Total_quotient_list[i*(2*Seats_total_init)+j].assigned  = true;
       }

    }
    //----- the j'th entry is now the party's votes divided by j+1;
  }
// each party's quotient list is now sorted individually, and the first "C" seats are
// "assigned", where C is the # of seats the party already won from Constituency races. 
// Total_quotient_list is stored as a block by party; below we will sort that (first
// by "assigned" status, and then by quotient value


// CHECK WHICH PARTY is the most OVER-REPRESENTED. --------------

int    most_OR_index=0;  // index of the Most over-represented party
double most_OR=0.0;
double current_rdiff;

for(i=0;i<Num_parties;i++)
  {
  current_rdiff  =  double(all_parties[i].seats_initial)/double(Seats_total_init) - (double(all_parties[i].votes/double(total_votes)) );

  if( current_rdiff > most_OR ) 
    {
    most_OR       = current_rdiff;
    most_OR_index = i;
    }
  }

all_parties[most_OR_index].most_over_represented=true;

// SORT the quotients list ----------

int array_size = Num_parties*2*Seats_total_init; // array of quotients to be ranked.

quotient temp1;
quotient temp2;

for (i=0; i<array_size; i++)
  {
  for (j=i+1; j<array_size; j++)
    {
    if( ( Total_quotient_list[j].value > Total_quotient_list[i].value ) || ( Total_quotient_list[j].assigned && !Total_quotient_list[i].assigned )  )
      {
      temp1                   = Total_quotient_list[i];
      Total_quotient_list[i]  = Total_quotient_list[j];
      Total_quotient_list[j]  = temp1;
      }
    }
  }
// Total_quotient_list IS NOW ORDERED first by "assigned" status, and then by "value" 
// it has size "array_size". 
// only the first 308 quotients are "assigned"

//  NOW ASSIGN SEATS IN ORDER 
int total_seats_assigned  = 0; 

// @@ check with gdb the status of these quotients

for (i=0; i<array_size; i++)
  {

  if ( should_terminate( Total_quotient_list, all_parties, i, Seats_total_init, total_seats_assigned, Seats_max_cutoff,  Num_parties, total_votes, count_NotA ) )     
     {
     break;
     } 

  if ( ! Total_quotient_list[i].assigned ) 
     {
     // @@ check gdb the first time this happens, i should be 308
     for(  j=0;j<Num_parties;j++)
       {
       if( Total_quotient_list[i].party_att == all_parties[j].name)
         {
         if( Total_quotient_list[i].party_att != "Oth")
           { //---- party 'other' doesn't get seats, obviously.
           
           all_parties[j].seats_assigned++;
           total_seats_assigned++;  
   
           }//--finished assigning to seat to party (provided it wasn't "Oth")
         }//--finished "if divisor list name matches"    
       }//--- finished scanning through the parties

     Total_quotient_list[i].assigned = true;
     }//--- finished checking if quotient is already assigned

  }//---finished scanning through the array of quotients (we should never get this far.)

finished_allocation:

// OUTPUT TO FILE 

for(i=0;i<Num_parties;i++)
  {
  datout <<  all_parties[i].name          << " \t "; 
  datout <<  all_parties[i].seats_initial << " \t ";  
  datout <<  all_parties[i].votes         << " \t ";
  datout <<  all_parties[i].vote_share    << " \t ";
  datout <<  all_parties[i].seats_assigned   << " \t ";
  datout <<  (double(all_parties[i].seats_assigned) / double(Seats_assigned_final)) << endl;
  }

// CLEAN UP MEMORY 
for(i=0;i<Num_parties;i++)
  {
  delete [] all_parties[i].party_quotient_list;
  }

// OUTPUT message to screen and terminate
cout << "\n ========================================= \n program complete.\n";
cout << " final number of seats assigned: " << Seats_assigned_final << " , majority threshold=" << double(Seats_assigned_final)/2.0
<< endl;

return 0;
} // End of main()


// =================   DEFINE FUNCTIONS   ===============

bool should_terminate ( quotient * Total_quotient_list, party * all_parties, int qi, int Seats_total_init , int & total_seats_assigned, int Seats_max_cutoff, int Num_parties, int total_votes , bool  count_NotA )
{
int i=0, j=0;
int total_seats_xcheck = 0;
bool underrep_found=false;
bool result = false;

// check if we've allocated all Const. seats yet. If not, keep going.
if ( qi < Seats_total_init )
  { 
    return(false); 
  }

// x-check total number of seats, and get the next candidate party
bool next_found=false;
int  next_index=-1; 
for(j=0;j<Num_parties;j++)
  {
  total_seats_xcheck  += all_parties[j].seats_assigned;
  if  ( all_parties[j].name ==  Total_quotient_list[qi].party_att )
    {
    if ( next_found )
       {
       cout << "\n ERROR: ambiguous party name attached ";
       cout << " to next candidate in should_terminate. Exiting.";
       exit(1);
       }
    next_found = true; // should only happen once
    next_index = j; 
    }
  }

if (  total_seats_xcheck != total_seats_assigned )
  {//sanity check.
  cout << "\n ERROR: inconsistent total seat count in should_terminate function.\n";
  exit(1);
  }

// check if we've reached cutoff 
if ( total_seats_assigned >= Seats_max_cutoff )
    {
    cout << "\n Parliament size reached cutoff threshold. Terminating at seat #";
    cout << total_seats_assigned;
    cout << " last seat assigned to the " << all_parties[j].name << " party.\n";
    return ( true ); 
    }

//========================================================= 

if ( count_NotA ) // counting "None of the Above" means each party's share  
  { // exit if no party remains under-represented by more than an integer 

  result = true;

  underrep_found = false;
    for(j=0;j<Num_parties;j++)
      {
      if ( ((all_parties[j].seats_assigned +1 )/total_seats_assigned) <  (all_parties[j].votes /total_votes) )
         { // in this case, under-representation of a party exceeds a full integer 
           // (meaning this party has the right to claim another full seat)  
           // therefore,  do not terminate seat allocation process. 
         result = false;
         break; 
         }
      } 
  }
else
  { //not counting NotA: fraction of votes *among major parties* -> fraction of seats
    // exit if the most overrepresented party has just been awarded a seat:

    result = false;

    if( all_parties[qi].seats_assigned >= all_parties[qi].seats_initial && all_parties[qi].most_over_represented)
       {
 
       result = true;

       all_parties[next_index].seats_assigned++;
       total_seats_assigned++;  

       }
  }  

return result;
}


