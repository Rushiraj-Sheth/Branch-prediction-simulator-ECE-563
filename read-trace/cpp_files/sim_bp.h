#ifndef SIM_BP_H
#define SIM_BP_H

#include <bits/stdc++.h>

using namespace std;

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
}bp_params;

// Put additional data structures here as per your requirement

long int totalNumOfRows = 1;
long int totalNumOfBHR ;
//long int index_id;
long int BHR = 0 ;


//Measurements
long int predictions = 0;
long int mispredictions = 0;

void initialise_BHT();

void predict_bimodal_gshare(int id,long int index, char outcome, long int N);
void bimodal_gshare_print(int id);
void update_bhr(bool acutal_outcome, long int N);

void predict_hybrid(long int N, char outcome, long int index_bimodal, 
                    long int index_gshare, long int index_chooser);

void hybrid_bimodal_gshare(int id, long int index, char local_prediction, char outcome, long int N);
void chooser_print();

vector<vector<long int>> BHT; 
vector<long int> chooser;
vector<long int> BHT_hybrid_gshare;

#endif
