#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TBox.h"
#include "TLegend.h"
#include "TH1D.h"
#include "TFile.h"
#include "TStyle.h"
#include "TColor.h"
#include "TH1F.h"

using namespace std;




struct Measurement
{
    double nch;         // charged-particle multiplicity
    double pt;          // pT bin centre
    double ptLow;       // lower pT bin edge
    double ptHigh;      // upper pT bin edge
    double yield;       // measured charged-particle yield

    double statPlus;    // positive statistical uncertainty
    double statMinus;   // negative statistical uncertainty

    double systPlus;    // positive systematic uncertainty
    double systMinus;   // negative systematic uncertainty
};


void pt_spectrum_comp()
{
    // ==========================================
    //  FILE LOCATION
  

    string inputFile  = R"(E:\Heavy ion project\Data\HEPData-ins2601281-v2-PbPb_2TeV_multPtSpec.csv)";

    string outputPNG  = R"(E:\Heavy ion project\Plots\compare_six_spectra.png)";

    string outputPDF  = R"(E:\Heavy ion project\Plots\compare_six_spectra.pdf)";

    string outputROOT = R"(E:\Heavy ion project\Results\compare_six_spectra.root)";


    // ==========================================
    //  SELECTING MULTIPLICITY CLASSES
  

    const int nClasses = 6;

    double selectedNch[nClasses] =
    {1, 10, 50, 100, 501, 1005};


    // COLOURS FOR SPECTRA

    int colors[nClasses] =
    {
        kBlue + 1,
        kRed + 1,
        kGreen + 2,
        kMagenta + 1,
        kOrange + 7,
        kCyan + 2
    };


    // ==========================================
    //  OPENING CSV FILE

    ifstream file(inputFile);

    if (!file.is_open())
    {
        cout << "ERROR: Cannot open CSV file!" << endl;
        return;
    }

    cout << "CSV opened successfully!" << endl;


    // STORING MEASUREMENT

    vector<Measurement> spectra[nClasses];

    string line;    


    // ==========================================
    //  READING CSV DATA
   
    while (getline(file, line))
    {
        
        if (line.empty() ||
            line[0] == '#' ||
            line[0] == '$')
            continue;


        stringstream ss(line);

        string value;
        vector<string> c;

        while (getline(ss, value, ','))
            c.push_back(value);

     

        if (c.size() < 11)
            continue;

        

        try
        {
            Measurement m;

           

            m.nch = stod(c[0]);          // table column 1  : Nch

            m.pt = stod(c[3]);           // table column 4  : pT
            m.ptLow = stod(c[4]);        // table column 5  : lower pT edge
            m.ptHigh = stod(c[5]);       // table column 6  : upper pT edge

            m.yield = stod(c[6]);        // table column 7  : yield

          

            m.statPlus = fabs(stod(c[7]));     // table column 8
            m.statMinus = fabs(stod(c[8]));    // table column 9

            m.systPlus = fabs(stod(c[9]));     // table column 10
            m.systMinus = fabs(stod(c[10]));   // table column 11

           
            for (int i = 0; i < nClasses; i++)
            {
                if (fabs(m.nch - selectedNch[i]) < 1e-6)
                {
                    spectra[i].push_back(m);
                    break;
                }
            }
        }
        catch (...)
        {
            continue;   
        }
    }

    file.close();


   

    cout << "\n========== SELECTED DATA ==========" << endl;

    int totalPoints = 0;

    for (int i = 0; i < nClasses; i++)
    {
        sort(
            spectra[i].begin(),
            spectra[i].end(),
            [](const Measurement &a,
               const Measurement &b)
            {
                return a.pt < b.pt;
            }
        );

        cout << "Nch = " << selectedNch[i]
             << " : "
             << spectra[i].size()
             << " measurements" << endl;

        totalPoints += spectra[i].size();
    }

    cout << "Total selected measurements: "
         << totalPoints << endl;


    

    if (totalPoints == 0)
    {
        cout << "ERROR: No selected classes found!" << endl;
        return;
    }


    // ==========================================
    // CREATING GRAPHS
   

    TGraphAsymmErrors *graphs[nClasses];

    double globalMin = 1e100;   
    double globalMax = 0;       

    for (int i = 0; i < nClasses; i++)
    {
        int n = spectra[i].size();      

        graphs[i] = new TGraphAsymmErrors(n);

       

        graphs[i]->SetName(
            ("spectrum_Nch_" + to_string((int)selectedNch[i])).c_str()
        );

       

        graphs[i]->SetMarkerStyle(20 + i);
        graphs[i]->SetMarkerSize(0.9);

        graphs[i]->SetMarkerColor(colors[i]);
        graphs[i]->SetLineColor(colors[i]);

        for (int j = 0; j < n; j++)
        {
            const Measurement &m = spectra[i][j];

            

            double xErrorLow = m.pt - m.ptLow;
            double xErrorHigh = m.ptHigh - m.pt;

            double yErrorLow = m.statMinus;
            double yErrorHigh = m.statPlus;

            graphs[i]->SetPoint(
                j,
                m.pt,
                m.yield
            );

            // SetPointError(index, exLow, exHigh, eyLow, eyHigh)

            graphs[i]->SetPointError(
                j,
                xErrorLow,
                xErrorHigh,
                yErrorLow,
                yErrorHigh
            );

           

            globalMin = min(
                globalMin,
                m.yield - m.systMinus
            );

            globalMax = max(
                globalMax,
                m.yield + m.systPlus
            );
        }
    }


    // ==========================================
    //  Y-AXIS RANGE
   

    double yMin = max(1e-9, globalMin * 0.5);

    double yMax = globalMax * 2.0;

    if (yMax <= yMin)
    {
        cout << "ERROR: Invalid plotting range!" << endl;
        return;
    }


    // ==========================================
    //  CREATING CANVAS
    

    TCanvas *canvas = new TCanvas(
        "canvas",
        "Comparison of ALICE multiplicity spectra",
        1200,
        850
    );

    canvas->SetLogy();
    canvas->SetLogx();

    canvas->SetLeftMargin(0.13);
    canvas->SetRightMargin(0.05);
    canvas->SetBottomMargin(0.13);
    canvas->SetTopMargin(0.08);


    // ==========================================
    //  DRAWING AXES
   

    TH1F *frame = canvas->DrawFrame(
        0.12,
        yMin,
        10.5,
        yMax
    );

    frame->SetTitle(
        "ALICE Pb-Pb collisions: Multiplicity comparison;"
        "Transverse momentum p_{T} (GeV/c);"
        "Charged-particle yield (GeV/c)^{-1}"
    );

    frame->GetXaxis()->SetTitleSize(0.045);
    frame->GetYaxis()->SetTitleSize(0.045);

    frame->GetXaxis()->SetLabelSize(0.04);
    frame->GetYaxis()->SetLabelSize(0.04);

    frame->GetXaxis()->SetTitleOffset(1.2);
    frame->GetYaxis()->SetTitleOffset(1.3);


    // ==========================================
    //  DRAWING SYSTEMATIC UNCERTAINTIES
   

    cout << "\nDrawing systematic uncertainties..." << endl;

    for (int i = 0; i < nClasses; i++)
    {
        for (const auto &m : spectra[i])
        {
            double x1 = m.ptLow;
            double x2 = m.ptHigh;

            double y1 = max(
                yMin,
                m.yield - m.systMinus
            );

            double y2 = m.yield + m.systPlus;

            TBox *box = new TBox(
                x1,
                y1,
                x2,
                y2
            );

            box->SetFillColorAlpha(colors[i], 0.22);

            box->SetLineColor(colors[i]);

            box->SetLineWidth(0);

            box->Draw("SAME");
        }
    }


    // ==========================================
    // DRAWING STATISTICAL UNCERTAINTIES
   

    cout << "Drawing statistical uncertainties..." << endl;

    for (int i = 0; i < nClasses; i++)
    {
        if (spectra[i].empty())
            continue;

        graphs[i]->Draw("P E1 SAME");
    }


  

    TLegend *legend = new TLegend(
        0.68,
        0.64,
        0.94,
        0.91
    );

    legend->SetBorderSize(1);
    legend->SetFillStyle(1001);
    legend->SetTextSize(0.030);

    for (int i = 0; i < nClasses; i++)
    {
        if (spectra[i].empty())
            continue;

        string label =
            "N_{ch} = " +
            to_string((int)selectedNch[i]);

        legend->AddEntry(
            graphs[i],
            label.c_str(),
            "lep"
        );
    }

    legend->Draw();


    // ==========================================
    //  SAVE OUTPUT
    

    canvas->SaveAs(outputPNG.c_str());

    canvas->SaveAs(outputPDF.c_str());



    TFile outFile(outputROOT.c_str(), "RECREATE");

    canvas->Write();

    for (int i = 0; i < nClasses; i++)
    {
        if (!spectra[i].empty())
            graphs[i]->Write();
    }

    outFile.Close();


    

    cout << "\n========== ANALYSIS COMPLETE ==========" << endl;

    cout << "Total selected measurements: "
         << totalPoints << endl;

    cout << "Statistical uncertainties: plotted as error bars" << endl;

    cout << "Systematic uncertainties: plotted as shaded boxes" << endl;

    cout << "PNG: " << outputPNG << endl;

    cout << "PDF: " << outputPDF << endl;

    cout << "ROOT: " << outputROOT << endl;

    cout << "=======================================" << endl;
}