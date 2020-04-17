#include "scanner.h"
#include <fstream>

int main(int argc, char* argv[]) {

    string filename_log;
    string filename_out;
    bool cpp = false;
    bool java = false;

    double fitnessThreshold = 0.0;
    int nDocs = 0;
    int nArt = 0;
    int execMs = 0;
    double prec= 0.0;
    double rec= 0.0;
    int nTopcs=0;
    int nIt=0;
    double fit=0.0;
    int nGA=0;
    int nLDA=0;
    int ldaAvgMs=0;
    string library;
    for (int arg = 0; arg < argc; arg++) {
        string s = argv[arg];
        if(s.compare("-log") == 0)
            filename_log = argv[++arg];
        if(s.compare("-out") == 0)
            filename_out = argv[++arg];
        else if(s.compare("-cpp") == 0)
            cpp = true;
        else  if(s.compare("-java")==0)
            java = true;
        else if(s.compare("-art")==0)
            nArt = stoi(argv[++arg]);
        else if(s.compare("-head")==0)
        cout<<"nDocs\tnArt\tfitT\tprec\trec\tnTopcs\tnIt\tfit\texecMs\tldaAvgMs\tnGA\tnLDA"<<endl;
    }

  ofstream outfile;
  outfile.open(filename_out, std::ios_base::app); // append instead of overwrite

Scanner sc(filename_log);
    if(cpp){
        // has 17 lines
        // interested on lines 3, 6, 7, 11, 12, 13, 14, 15, 16, 17
//        sc.nextLineClean();
        sc.nextLineClean();
        fitnessThreshold = sc.nextDouble();
        sc.nextLineClean();
        sc.nextLineClean();
        sc.nextLineClean();
        sc.nextWord();
        library = sc.nextWord();
        sc.nextLineClean();
        nDocs = sc.nextInt();
        sc.nextLineClean();
        nArt = sc.nextInt();
        sc.nextLineClean();
        sc.nextLineClean();
        sc.nextLineClean();
        sc.nextLineClean();
        execMs = sc.nextInt();
        sc.nextLineClean();
        prec = sc.nextDouble();
        sc.nextLineClean();
        rec = sc.nextDouble();
        sc.nextLineClean();
        nTopcs = sc.nextInt();
        nIt = sc.nextInt();
        fit = sc.nextDouble();
        sc.nextLineClean();
        nGA = sc.nextInt();
        sc.nextLineClean();
        nLDA = sc.nextInt();
        sc.nextLineClean();
        ldaAvgMs = sc.nextInt();


        outfile<<nDocs           <<"\t"
            <<nArt            <<"\t"
            <<fitnessThreshold<<"\t"
            <<prec            <<"\t"
            <<rec             <<"\t"
            <<nTopcs          <<"\t"
            <<nIt             <<"\t"
            <<fit             <<"\t"
            <<execMs          <<"\t"
            <<ldaAvgMs        <<"\t"
            <<nGA             <<"\t"
            <<nLDA            <<"\t"
            <<library<<endl;

    } else if (java){
        // 2 lines
        fitnessThreshold = sc.nextDouble();
        sc.nextLineClean();
        sc.nextInt();
        execMs = sc.nextInt();
        prec = sc.nextDouble();
        rec = sc.nextDouble();
        nTopcs = sc.nextInt();
        nIt = sc.nextInt();
        fit = sc.nextDouble();
        nLDA = sc.nextInt();
        ldaAvgMs = sc.nextInt();


        outfile<<nDocs           <<"\t"
            <<nArt            <<"\t"
            <<fitnessThreshold<<"\t"
            <<prec            <<"\t"
            <<rec             <<"\t"
            <<nTopcs          <<"\t"
            <<nIt             <<"\t"
            <<fit             <<"\t"
            <<execMs          <<"\t"
            <<ldaAvgMs        <<"\t"
            <<nGA             <<"\t"
            <<nLDA            <<endl;
    }

    sc.close();
    outfile.close();

}

