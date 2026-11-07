#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TColor.h"
#include "TROOT.h"

using namespace std;


void pt_spectrum()
{
    // =====================================================================
    // FILE LOCATIOM

    string inputFile  = R"(E:\Heavy ion project\Data\HEPData-ins2601281-v2-PbPb_2TeV_multPtSpec.csv)";
    string outputFile = R"(E:\Heavy ion project\Plots\pt_spectrum_Nch1.png)";


    // =====================================================================
    // OPENING THE CSV FILE
   

    ifstream file(inputFile);

    if (!file.is_open())
    {
        cout << "ERROR: Cannot open CSV file!" << endl;
        return;
    }

    cout << "CSV file opened successfully!" << endl;


    // =====================================================================
    //  CREATING THE TWO GRAPHS
    //    one for statistcial and other for Systematic uncertainties

    TGraphAsymmErrors *gStat = new TGraphAsymmErrors();
    TGraphAsymmErrors *gSyst = new TGraphAsymmErrors();

    string line;        // one line of the CSV at a time
    int    point = 0;   // index of the next point to add to the graphs


    // =====================================================================
    //  READING THE CSV LINE BY LINE
    

    while (getline(file, line))
    {
        // Skip empty lines, metadata lines (#, lines 1-10) Data starts at line 12.

        if (line.empty() || line[0] == '#' || line[0] == '$')
            continue;

        
        stringstream   ss(line);
        string         value;
        vector<string> columns;

        while (getline(ss, value, ','))
        {
            columns.push_back(value);
        }

        // A valid data line needs at least 11 columns (indices 0 to 10).

        if (columns.size() < 11)
            continue;

        

        try
        {
            // -------------------------------------------------------------
            //  EXTRACT THE MEASUREMENTS
            //    (index in code = column number in the HEPData table - 1)
            //
            //    columns[0]  : N_ch                    (table column 1)
            //    columns[3]  : pT                      (table column 4)
            //    columns[4]  : lower pT bin edge       (table column 5)
            //    columns[5]  : upper pT bin edge       (table column 6)
            //    columns[6]  : yield                   (table column 7)
            //    columns[7]  : +ve statistical error   (table column 8)
            //    columns[8]  : -ve statistical error   (table column 9)
            //    columns[9]  : +ve systematic error    (table column 10)
            //    columns[10] : -ve systematic error    (table column 11)
            //

            double nch = stod(columns[0]);

            // Keep only the first multiplicity class (Nch = 1).
            

            if (fabs(nch - 1.0) > 1e-9)
                continue;

            double pt     = stod(columns[3]);
            double ptLow  = stod(columns[4]);
            double ptHigh = stod(columns[5]);

            double yield  = stod(columns[6]);

            // Statistical uncertainties 

            double statPlus  = fabs(stod(columns[7]));
            double statMinus = fabs(stod(columns[8]));

            // Systematic uncertainties 

            double systPlus  = fabs(stod(columns[9]));
            double systMinus = fabs(stod(columns[10]));


            // -------------------------------------------------------------
            //  STORING THE STATISTICAL ERRORS
            //  SetPointError(index, exLow, exHigh, eyLow, eyHigh)
           

            gStat->SetPoint(point, pt, yield);

            gStat->SetPointError(
                point,
                0.0,
                0.0,
                statMinus,
                statPlus
            );


            // -------------------------------------------------------------
            //  STORING THE SYSTEMATIC ERRORS
           

            gSyst->SetPoint(point, pt, yield);

            gSyst->SetPointError(
                point,
                pt - ptLow,
                ptHigh - pt,
                systMinus,
                systPlus
            );

            point++;    
        }
        catch (...)
        {
            continue;   
        }
    }

    file.close();



    

    cout << "Number of measurements selected: " << point << endl;

    if (point == 0)
    {
        cout << "ERROR: No data points found!" << endl;
        return;
    }


    // =====================================================================
    //    CREATING THE CANVAS
   
   

    TCanvas *canvas = new TCanvas(
        "canvas",
        "ALICE Transverse Momentum Spectrum",
        1000,
        750
    );

    canvas->SetLogy();


    // =====================================================================
    //  DRAWING THE SYSTEMATIC UNCERTAINTIES (FIRST)


    gSyst->SetTitle(
        "ALICE Pb-Pb collisions, Nch = 1;"
        "p_{T} (GeV/c);"
        "Charged-particle yield (GeV/c)^{-1}"
    );

    gSyst->SetMarkerStyle(20);

    gSyst->SetFillColorAlpha(kOrange - 3, 0.35);   
    gSyst->SetLineColor(kOrange + 1);

    gSyst->SetMinimum(1e-7);    
    gSyst->SetMaximum(0.1);     
    gSyst->Draw("A2");


    // =====================================================================
    //     DRAWING THE STATISTICAL UNCERTAINTIES ON TOP
   

    gStat->SetMarkerStyle(20);
    gStat->SetMarkerSize(0.8);

    gStat->SetMarkerColor(kBlue + 2);
    gStat->SetLineColor(kBlue + 2);

    gStat->Draw("P E1 SAME");


  

    TLegend *legend = new TLegend(0.60, 0.75, 0.88, 0.88);

    legend->AddEntry(gStat, "Statistical uncertainty",  "lep");
    legend->AddEntry(gSyst, "Systematic uncertainty",   "f");

    legend->Draw();


    // =====================================================================
    //  SAVING THE PLOT AS PNG
   

    canvas->SaveAs(outputFile.c_str());

    cout << "Graph saved successfully!" << endl;
    cout << "Output: " << outputFile << endl;
}