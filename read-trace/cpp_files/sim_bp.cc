#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_bp.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        params.N        = 0 ;
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    char str[2];

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BIMODAL/GSHARE CONFIG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if(strcmp(params.bp_name, "bimodal") == 0)totalNumOfRows = pow(2,params.M2);
    else if(strcmp(params.bp_name, "gshare") == 0)totalNumOfRows = pow(2,params.M1);

    totalNumOfBHR = 1;
    BHT.resize(totalNumOfRows, vector<long int>(totalNumOfBHR));
   // cout<<"initialise done"<<endl;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */ 
     
    if(strcmp(params.bp_name, "hybrid") == 0){
        chooser.resize( pow(2,params.K), -1 );
        BHT.resize( pow(2,params.M2) , vector<long int>(totalNumOfBHR));
        totalNumOfRows = pow(2,params.M2);
        BHT_hybrid_gshare.resize( pow(2, params.M1), -1 );
    }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    initialise_BHT();

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        
        outcome = str[0];
        if (outcome == 't'){}
            //printf("%lx %s\n", addr, "t");           // Print and test if file is read correctly
        else if (outcome == 'n'){}
            //printf("%lx %s\n", addr, "n");          // Print and test if file is read correctly
        /*************************************
            Add branch predictor code here
        **************************************/

        long int PC = (addr >> 2);

       //bimodal logic
       if(strcmp(params.bp_name, "bimodal") == 0){
            
            long int mask = (1 << params.M2) - 1;
            long int index_id = PC & mask;
            predict_bimodal_gshare(0,index_id,outcome,params.N);
       }

        //gshare logic
        else if(strcmp(params.bp_name, "gshare") == 0){

            long int mask = (1<<params.M1) - 1;
            long int required_bits = PC & mask ;
            
            long int index_id = (BHR << (params.M1 - params.N) ) ^ (required_bits);
            predict_bimodal_gshare(1,index_id,outcome,params.N);
           

        }
        else{
            //for bimodal BHT
            long int mask = (1 << params.M2) - 1;
            long int index_bimodal = PC & mask; 

            //for ghsare BHT
            mask = (1<<params.M1) - 1;
            long int required_bits = PC & mask ;
            long int index_gshare = (BHR << (params.M1 - params.N) ) ^ (required_bits);

            //for chooser table
            mask = (1 << params.K) - 1;
            long int index_chooser = mask & PC;

            predict_hybrid(params.N, outcome, index_bimodal, index_gshare, index_chooser);


        }





    }

    if(strcmp(params.bp_name, "bimodal") == 0){
        bimodal_gshare_print(0);
    }
    else if(strcmp(params.bp_name, "gshare") == 0){
       bimodal_gshare_print(1);
    }
    else{
        chooser_print();
    }


    return 0;
}



void initialise_BHT(){
    for(int i=0;i<totalNumOfRows;i++){
        for(int j=0;j<totalNumOfBHR;j++){
            BHT[i][j] = -1;
        }
    }
}

void bimodal_gshare_print(int id){
    
    //id = 0 ->bimodal
    //id = 1 ->gshare

    cout<<"OUTPUT"<<endl;
    cout<<"number of predictions:    "<<predictions<<endl;
    cout<<"number of mispredictions: "<<mispredictions<<endl;
    cout<<"misprediction rate:       "<<fixed<<setprecision(2)<<(double)((double)mispredictions/predictions)*(100)<<"%"<<endl;
    if(id == 0)
        cout<<"FINAL BIMODAL CONTENTS"<<endl;
    else cout<<"FINAL GSHARE CONTENTS"<<endl;

/*
 
    for(int i=0;i<totalNumOfRows;i++){
        for(int j=0;j<totalNumOfBHR;j++){
            if(BHT[i][j] != -1){
                cout<<i<<"     ";
                cout<<BHT[i][j];
            }
            else{
                cout<<i<<"     ";
                cout<<2;
            }
        }
        
        cout<<endl;        

    }
*/

}


void chooser_print(){
    cout<<"OUTPUT"<<endl;
    cout<<"number of predictions:    "<<predictions<<endl;
    cout<<"number of mispredictions: "<<mispredictions<<endl;
    cout<<"misprediction rate:       "<<fixed<<setprecision(2)<<(double)((double)mispredictions/predictions)*(100)<<"%"<<endl;
    cout<<"FINAL CHOOSER CONTENTS"<<endl;

    for(long int i=0;i<chooser.size();i++){
        if(chooser[i]!=-1){
            cout<<i<<"     ";
            cout<<chooser[i]<<endl;
        }
        else{
            cout<<i<<"    ";
            cout<<1<<endl;
        }

    }

    cout<<"FINAL GSHARE CONTENTS"<<endl;
    for(long int i=0;i<BHT_hybrid_gshare.size();i++){
        if(BHT_hybrid_gshare[i]!=-1){
            cout<<i<<"     ";
            cout<<BHT_hybrid_gshare[i]<<endl;
        }
        else{
            cout<<i<<"     ";
            cout<<2<<endl;
        }

    }

    cout<<"FINAL BIMODAL CONTENTS"<<endl;
    //int flg=0;
    for(int i=0;i<totalNumOfRows;i++){
      //  flg=0;
        for(int j=0;j<totalNumOfBHR;j++){
            if(BHT[i][j] != -1){
        //        flg = 1;
                cout<<i<<"     ";
                cout<<BHT[i][j];
            }
            else{
                cout<<i<<"    ";
                cout<<2;
            }
        }

        //if(flg){
            cout<<endl;
        //}

    }


}



void predict_bimodal_gshare(int id,long int index, char outcome, long int N)
{
    predictions++;
    char local_prediction;

    //id = 1 ->gshare
    //id = 0 -> bimodal

    if(BHT[index][0] == -1)BHT[index][0] = 2;

    if( (BHT[index][0] == 2) || (BHT[index][0] == 3) ){
        local_prediction = 't';
    }
    else{
        local_prediction = 'n';
    }

    if(local_prediction != outcome){
        mispredictions++;

        if(outcome == 't'){
            //our predicton is not taken. hence counter = 0 or 1
            //so we correct and increment
            BHT[index][0]++;

            if(id == 1){
                update_bhr(1, N);
            }
        }
        else{
            //our prediction is taken and actual is not taken. so current counter = 2 or 3
            BHT[index][0]--;

            if(id == 1){
                update_bhr(0, N);
            }
        }
    }
    else{
        //prediction is correct
        //case 1: outcome is taken
        if(outcome == 't'){
            //current counter = 2 or 3
            (BHT[index][0] == 3)?(BHT[index][0]=3) : (BHT[index][0]+=1);

            if(id == 1){
                update_bhr(1, N);
            }

        }
        //case 2: outcome is not-taken
        else{
            //counter is 0 or 1
            if(BHT[index][0]==1)BHT[index][0]--;

            if(id == 1){
                update_bhr(0, N);
            }


        }
    }

}

void update_bhr(bool acutal_outcome, long int N){

    //outcome = 1 -> taken
    //outcome = 0 -> not-taken
    if(N>0){
        //shifting right
        unsigned long int temp_bhr = (BHR >> 1) ;
        unsigned long int bhr_mask = ( 1 << (N-1) );

        //update the BHR
        if( acutal_outcome ){
            //if taken
            BHR = temp_bhr | bhr_mask; 
        }
        else{
            // not taken
            BHR = temp_bhr;
        }

    }
    
}


void predict_hybrid(long int N, char outcome, long int index_bimodal, 
                    long int index_gshare, long int index_chooser){
    
    predictions++;

    //getting bimodal and gshare prediction
    char bimodal_pred, gshare_pred;
    if(BHT[index_bimodal][0] == -1){
        BHT[index_bimodal][0] = 2;
    }

    if(BHT[index_bimodal][0]>=2){
        //predict taken
        bimodal_pred = 't';
    }
    else{
        bimodal_pred = 'n';
    }

    //GETTING GHSARE PREDICTION
    if(BHT_hybrid_gshare[index_gshare] == -1){
        BHT_hybrid_gshare[index_gshare] = 2;
    }

    if(BHT_hybrid_gshare[index_gshare] >= 2){
        gshare_pred = 't';
    }
    else{
        gshare_pred = 'n';
    }

    //chooser table
    if(chooser[index_chooser] == -1){
        chooser[index_chooser] = 1;
    }

    if(chooser[index_chooser] >= 2){
        //use gshare prediction
        hybrid_bimodal_gshare(1,index_gshare,gshare_pred, outcome, N );
    }
    else{
        //bimodal prediction
        hybrid_bimodal_gshare(0, index_bimodal, bimodal_pred, outcome, N );
    }

    if( (gshare_pred == outcome) && (bimodal_pred != outcome) ){
        //mispredictions++;
        if(chooser[index_chooser] <=2 )chooser[index_chooser]++;
    }
    else if((gshare_pred != outcome) && (bimodal_pred == outcome)){
        //mispredictions++;
        if(chooser[index_chooser] >=1 )chooser[index_chooser]--;
    }


}

void hybrid_bimodal_gshare(int id, long int index, char local_prediction, char outcome, long int N){

    //id = 0 -> bimodal
    //id = 1 -> gshare
        
    if(local_prediction != outcome){
        
        mispredictions++;

        if(outcome == 't'){
            //our predicton is not taken. hence counter = 0 or 1
            //so we correct and increment
            if(id == 0){
             BHT[index][0]++;
            }
            else{
                BHT_hybrid_gshare[index]++;
            }

            update_bhr(1, N);

        }
        else{
            //our prediction is taken and actual is not taken. so current counter = 2 or 3
           if(id == 0){
                BHT[index][0]--;
           }
           else{
            BHT_hybrid_gshare[index]--;
           }

            update_bhr(0, N);
        }
    }
    else{
        //prediction is correct
        //case 1: outcome is taken
        if(outcome == 't'){
            //current counter = 2 or 3
            if(id == 0){
                (BHT[index][0] == 3)?(BHT[index][0]=3) : (BHT[index][0]+=1);
            }
            else{
                if(BHT_hybrid_gshare[index] == 2){
                    BHT_hybrid_gshare[index]++;
                }
            }

            update_bhr(1, N);
        }
        //case 2: outcome is not-taken
        else{
            //counter is 0 or 1
            if(id == 0)
            {
                if(BHT[index][0]==1)BHT[index][0]--;
            }
            else{
                if(BHT_hybrid_gshare[index] == 1)BHT_hybrid_gshare[index]--;
            }

            update_bhr(0, N);

        }
    }
}