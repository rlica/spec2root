	  /**************************************
	  * Spec2Root - Analyse spectrum files  *
	  *             with ROOT               *
	  *                                     *
	  * Author: R. Lica, razvan.lica@cern.ch* 
	  * 13-Jul-2015                         *
	  *                                     *
	  * Compile and run with:               *
	  *    make clean && make && ./spec2root*
	  *                                     *
	  * Prerequisites: ROOT                 *
	  *                                     *
	  **************************************/

//Changelog:
//15 Dec 2016 - expo+pol0
//05 Aug 2016 - binned likelyhood
//13 Jul 2015 - expo fit


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "TFile.h"
#include "Riostream.h"
#include "TTree.h"
#include "TLine.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TText.h"
#include "TROOT.h"
#include "TApplication.h"
#include "TH1.h"
#include "TF1.h"
#include "TStyle.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"

using namespace std;

//#define DATATYPE float
#define DATATYPE int

  DATATYPE *data;

  int nchan,
      graphical,
      time_spec,
      gauss_fit,
      expo_fit,
      conv_fit,
      ngates,
      bkg_iterations,
      gateL[1000],
      gateR[1000];

  
  double gBinW = 1;
  float	 conv_centroid=-1.0,
         conv_fwhm=-1.0;

  FILE *file1, 
       *file2;

  char outfile_ascii[100],
       outfile_gates[100],
       *files[3],
       answer[10], 
       *option_string,
       date_and_time[26];
    


void timer() {

	time_t timer;
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(date_and_time, 26, "%d/%m/%Y %H:%M:%S", tm_info);
    puts(date_and_time);
    

	}

void interactive() {
	
	  
  printf("\n\nNumber of channels (1 - 16 k): ");
  scanf("%d", &nchan);
  if (nchan>=1 && nchan<=16) {
    nchan=nchan*1024;
  }
  else {
    printf(" ERROR - wrong number of channels: %d\n", nchan);
    exit(0);
  }
  
  printf("Display spectrum in ROOT? (y/n): ");
  scanf("%s", &answer);
  if      ( strncmp(answer, "y", 1) == 0 )  graphical = 1;
  else if ( strncmp(answer, "n", 1) == 0 ) graphical = 0;
  else {
    printf(" ERROR - wrong answer\n\n");
    exit(0);
  }

  printf("Time or energy spectrum? (t/e): ");
  scanf("%s", &answer);
  if      ( strncmp(answer, "t", 1) == 0 )  time_spec = 1;
  else if ( strncmp(answer, "e", 1) == 0 )  time_spec = 0;
  else {
    printf(" ERROR - wrong answer\n\n");
    exit(0);
  }



  if (time_spec == 0) {
	      
      printf("Subtract background automatically? (y/n): ");
      scanf("%s", &answer);
      if ( strncmp(answer, "n", 1) == 0 ) ;
      else if ( strncmp(answer, "y", 1) == 0 )  { 
	        printf("HighRes (HPGe) or LowRes (LaBr3:Ce)? (h/l): ");
	    	scanf("%s", &answer);
	    	if ( strncmp(answer, "h", 1) == 0 )  { 
	    	bkg_iterations = 8;									//Tuning background fitting for HPGe
	    	option_string = "same"; 
	    	}
	    	else if ( strncmp(answer, "l", 1) == 0 ) { 
	    		bkg_iterations = 90;							//Tuning background fitting for LaBr
	    		option_string = "same BackOrder4 nosmoothing "; 
	    	}
	    	else {
	    	printf(" ERROR - wrong answer\n\n");
	    	exit(0);
	    	}

	    	if (ngates > 0) printf("Fit gaussian instead of integrating? (y/n): ");
                scanf("%s", &answer);
                if      ( strncmp(answer, "y", 1) == 0 ) gauss_fit = 1;
                else if ( strncmp(answer, "n", 1) == 0 ) gauss_fit = 0;
                else {
                  printf(" ERROR - wrong answer\n\n");
                  exit(0);
                } 
	    	}
      
      else {
          printf(" ERROR - wrong answer\n\n");
          exit(0);
      }




  }

  else {
	  printf("Fit gaussian, exponential, convolution or skip fitting? (g/e/c/s): ");
      scanf("%s", &answer);
      if      ( strncmp(answer, "s", 1) != 0 ) {

		  if  ( strncmp(answer, "g", 1) == 0 ) gauss_fit = 1;
		  else if  ( strncmp(answer, "e", 1) == 0 ) expo_fit  = 1;
  		  else if  ( strncmp(answer, "c", 1) == 0 ) { 
			  conv_fit  = 1;
			  printf("Fixed centroid = (0 for free): ");
			  scanf("%f", &conv_centroid);
			  printf("Fixed FWHM = (0 for free): ");
			  scanf("%f", &conv_fwhm);
		  }
			  
			  
			  
		  else { printf("ERROR - wrong answer\n\n"); exit(0); }
		  
		  printf("Input region for fit: ");
          scanf("%d %d", &gateL[0], &gateR[0]);
          if (gateL[0] < 0 || gateR[0] < 0 || gateL[0] > gateR[0] ) {
             printf(" ERROR - wrong answer: %d %d\n\n", gateL[0], gateR[0]);
             exit(0);
          } 
      }		  
 
      else if ( strncmp(answer, "s", 1) == 0 ) { gauss_fit = 0; expo_fit = 0; conv_fit = 0; }
      else {
        printf(" ERROR - wrong answer\n\n");
        exit(0);
      }
	
  }

}

void open_files(int argc, char **argv) {
	
  	
  if( argc == 2) {
	  file1 = fopen( argv[1], "rb");
	  	  
  }
  else if( argc == 3) { 
	  file1 = fopen( argv[1], "rb"); 
	  file2 = fopen( argv[2], "rt");
	  
  }
  else {
    printf(" Usage: spec2root [spectrum(binary, integer/float)] [gates(optional, ascii)] \n");
    printf(" The gates file should have 2 columns containing left and right limits for integration.\n");
    exit(0);
  }
  
  if( file1 == NULL) {
    printf("  ERROR - cannot open file : %s\n",argv[1]);
    exit(0);
  }
  else {
	  printf("File '%s' OK\n", argv[1]);
	  files[1] = argv[1];
  } 
  
  
  if( file2 == NULL) {
    printf("No gates will be set for integration\n");
  }
  else {
	  printf("File '%s' OK", argv[2]);
	  files[2] = argv[2];
  }
	
	
}

int read_gates() {
	
	int i=0;
	if (time_spec == 1) return 0;
	if (file2 == NULL) return 0;
	while (fscanf(file2, "%d %d", &gateL[i], &gateR[i]) != EOF ) i++;
	if (i==0) {
		printf(" ----> No limits found\n");
		return 0;
	}
	else {
	    printf(" ----> %d limits read \n", i);
		return i;
	}
	
}

void fit_gauss(TH1F *hist, int gateL, int gateR) {

  double gConstant = 1;
  double gSigma = 0.32 * (gateR - gateL);
  double gSigma_error;
  double gArea = hist->Integral(gateL, gateR);  
  double gMean = (gateR-gateL)/2 ;
  double gChi2pNDF = 1.;
  double gFWHM;
  double gArea_error = sqrt(gArea);
  double gFWHM_error; 
  double gMean_error;
  double gConstant_error;
   
  static Float_t sqrt2pi = TMath::Sqrt(2*TMath::Pi());
  
  FILE *out = fopen("results.txt", "at");
  fprintf(out, "'%s' \t %s ",files[1], &date_and_time);
      
      

  //gaus = A gaussian with 3 parameters: f(x) = p0*exp(-0.5*((x-p1)/p2)^2)
  //p1->GetParameter(0);//this is the constant
  //p1->GetParameter(1);//this is the mean
  //p1->GetParameter(2); //this is the sigma
 
  TF1 *fitfunc = new TF1("gauss","gaus", gateL, gateR);
  
  fitfunc->SetParameters(gConstant, gMean, gSigma);
  fitfunc->SetLineColor(3);
  fitfunc->SetLineWidth(1);
  
  hist->Fit(fitfunc,"+Q","same",gateL, gateR);
  
  gConstant   		= fitfunc->GetParameter(0);
  gConstant_error   = fitfunc->GetParError(0);
  gSigma      		= fitfunc->GetParameter(2);
  gSigma_error 		= fitfunc->GetParError(2);
  gMean       		= fitfunc->GetParameter(1);
  gMean_error 		= fitfunc->GetParError(1);
  gFWHM       		= 2.35482*gSigma;
  gFWHM_error 		= 2.35482*fitfunc->GetParError(2);
  gArea		  		= gConstant * gSigma * sqrt2pi; 
  gArea_error 		= (gConstant_error * gSigma + gConstant * gSigma_error) * sqrt2pi; 
  
  gChi2pNDF   = fitfunc->GetChisquare() / fitfunc->GetNDF();
  
  
 
 	
  fprintf(out, " \t %4d-%4d  Centr=%8.3f(%.3f) A=%9.1f(%.3f) FWHM=%5.1f(%.3f) Chi2/NDF=%2.1f\n",
                 gateL, gateR, gMean, gMean_error, gArea, gArea_error, gFWHM, gFWHM_error,gChi2pNDF);
    printf(    " \t %4d-%4d  Centr=%8.3f(%.3f) A=%9.1f(%.3f) FWHM=%5.1f(%.3f) Chi2/NDF=%2.1f\n",
	             gateL, gateR, gMean, gMean_error, gArea, gArea_error, gFWHM, gFWHM_error,gChi2pNDF);

	
	fclose(out);


	}


void fit_expo(TH1F *hist, int gateL, int gateR) {

  double eConstant = 1;
  
  double eConstant_error;
  double eChi2pNDF = 1.;
  double eSlope;
  double eSlope_error;

  double halflife;
  double halflife_error;
   
  FILE *out = fopen("results.txt", "at");
  fprintf(out, "'%s' \t %s ",files[1], &date_and_time);
       

  //expo = An exponential with 2 parameters: f(x) = exp(p0+p1*x).
  //p1->GetParameter(0);//this is the constant
  //p1->GetParameter(1);//this is the slope
 
  TF1 *fitfunc = new TF1("exponential","expo", gateL, gateR);
  
  fitfunc->SetLineColor(3);
  fitfunc->SetLineWidth(1);
  
  //hist->Fit(fitfunc,"+Q","same",gateL, gateR);       //high statistics - chisq fit
  hist->Fit(fitfunc,"+Q L","same",gateL, gateR);   //low statistics - likelihood
  
  eConstant   		= fitfunc->GetParameter(0);
  eConstant_error       = fitfunc->GetParError(0);
  eSlope      		= fitfunc->GetParameter(1);
  eSlope_error 		= fitfunc->GetParError(1);
  
  eChi2pNDF   = fitfunc->GetChisquare() / fitfunc->GetNDF();

  halflife       = 0.6931471806/(-eSlope);
  halflife_error = 0.6931471806/(eSlope*eSlope)*eSlope_error;
 	
  fprintf(out, " \t %4d-%4d  T(1/2)=%8.3f(%.3f) Chi2/NDF=%2.1f\n",
                 gateL, gateR, halflife, halflife_error, eChi2pNDF);
    printf(    " \t %4d-%4d  T(1/2)=%8.3f(%.3f) Chi2/NDF=%2.1f\n",
                 gateL, gateR, halflife, halflife_error, eChi2pNDF);
	
	fclose(out);


	}
	
void fit_expo_pol0(TH1F *hist, int gateL, int gateR) {

  double eConstant = 1;
  double eConstant_error;
  double eChi2pNDF = 1.;
  double eSlope;
  double eSlope_error;

  double halflife;
  double halflife_error;
   
  FILE *out = fopen("results.txt", "at");
  fprintf(out, "'%s' \t %s ",files[1], &date_and_time);
       

  //expo = An exponential with 2 parameters: f(x) = exp(p0+p1*x).
  //p1->GetParameter(0);//this is the constant
  //p1->GetParameter(1);//this is the slope
 
  TF1 *fitfunc = new TF1("exponential","expo(0)+pol0(2)", gateL, gateR);
  
  fitfunc->SetLineColor(3);
  fitfunc->SetLineWidth(2);
  
  TText *label = new TText();
  label->SetTextFont(43);
  label->SetTextSize(20);
  label->SetNDC();
   
  
  
  
  hist->Fit(fitfunc,"+Q","same",gateL, gateR);       //high statistics - chisq fit
  
  eConstant   		= fitfunc->GetParameter(0);
  eConstant_error       = fitfunc->GetParError(0);
  eSlope      		= fitfunc->GetParameter(1);
  eSlope_error 		= fitfunc->GetParError(1);
  
  eChi2pNDF   = fitfunc->GetChisquare() / fitfunc->GetNDF();

  halflife       = 0.6931471806/(-eSlope);
  halflife_error = 0.6931471806/(eSlope*eSlope)*eSlope_error;
 	
  fprintf(out, " \t %4d-%4d  T(1/2)=%8.3f(%.3f) BG=%8.3f(%.3f) Chi2/NDF=%2.1f ",
                 gateL, gateR, halflife, halflife_error, fitfunc->GetParameter(2), fitfunc->GetParError(2), eChi2pNDF);
    printf(    " \t %4d-%4d  T(1/2)=%8.3f(%.3f) BG=%8.3f(%.3f) Chi2/NDF=%2.1f ",
                 gateL, gateR, halflife, halflife_error, fitfunc->GetParameter(2), fitfunc->GetParError(2), eChi2pNDF);
		
  label -> SetTextColor(3);
  label -> DrawText(.4, .7,  Form("Chi-squared             T(1/2) = %8.3f(%.3f)", halflife, halflife_error));
		
	
		
	
  fitfunc->SetLineColor(2);
  fitfunc->SetLineWidth(2);
	
  hist->Fit(fitfunc,"+Q L","same",gateL, gateR);   //low statistics - likelihood
  
  eConstant   		= fitfunc->GetParameter(0);
  eConstant_error       = fitfunc->GetParError(0);
  eSlope      		= fitfunc->GetParameter(1);
  eSlope_error 		= fitfunc->GetParError(1);
  
  eChi2pNDF   = fitfunc->GetChisquare() / fitfunc->GetNDF();

  halflife       = 0.6931471806/(-eSlope);
  halflife_error = 0.6931471806/(eSlope*eSlope)*eSlope_error;
 	
  fprintf(out, " \t  T(1/2)(b.l.)=%8.3f(%.3f) BG=%8.3f(%.3f) Chi2/NDF=%2.1f \n",
                 gateL, gateR, halflife, halflife_error, fitfunc->GetParameter(2), fitfunc->GetParError(2), eChi2pNDF);
    printf(    " \t  T(1/2)(b.l.)=%8.3f(%.3f) BG=%8.3f(%.3f) Chi2/NDF=%2.1f \n",
                 gateL, gateR, halflife, halflife_error, fitfunc->GetParameter(2), fitfunc->GetParError(2), eChi2pNDF);
  
  
  label -> SetTextColor(2);
  label -> DrawText(.4, .75, Form("Binned Likelyhood  T(1/2) = %8.3f(%.3f)", halflife, halflife_error));
	
	
	
	
  fclose(out);


	}	


void fit_conv(TH1F *hist, int gateL, int gateR) {


  
  double Chi2pNDF = 1.;
  
  double Integral = 1;
  double Integral_error;
  
  double Lifetime;
  double Lifetime_error;

  double halflife;
  double halflife_error;

  double Centroid;
  double Centroid_error;

  double Sigma;
  double Sigma_error;


  
  TText *label = new TText();
  label->SetTextFont(43);
  label->SetTextSize(20);
  label->SetNDC();
   
   
  FILE *out = fopen("results.txt", "at");
  fprintf(out, "'%s' \t %s ",files[1], &date_and_time);

 
  //First fit with gaussian to find initial parameters:
  
  double gConstant = 1;
  double gSigma = 0.32 * (gateR - gateL);
  double gArea = hist->Integral(gateL, gateR);  
  double gMean = (gateR-gateL)/2 ;
  static Float_t sqrt2pi = TMath::Sqrt(2*TMath::Pi());
  
  TF1 *fitfunc0 = new TF1("gauss","gaus", gateL, gateR);
  fitfunc0->SetParameters(gConstant, gMean, gSigma);
  fitfunc0->SetLineColorAlpha(4,0);
  fitfunc0->SetLineWidth(1);
  
  hist->Fit(fitfunc0,"+Q","same",gateL, gateR);
  
  gConstant		    = fitfunc0->GetParameter(0);
  gSigma      		= fitfunc0->GetParameter(2);
  gMean       		= fitfunc0->GetParameter(1);
  gArea		  		= gConstant * gSigma * sqrt2pi; 

  
  
  //conv = Convolution of gaussian with exponential 4 parameters
  //p1->GetParameter(0);//this is the integral
  //p1->GetParameter(1);//this is the lifetime
  //p1->GetParameter(2);//this is the centroid
  //p1->GetParameter(3);//this is the sigma
  // TAKEN FROM: 
  
    
  TF1 *fitfunc = new TF1("conv","0.5*([0]/[1])*exp(-(x-[2])/[1])*exp(([3]*[3])/(2*[1]*[1]))*(1-TMath::Erf(([3]/(((2^(0.5))*[1])))-((x-[2])/(((2^(0.5))*[3])))))", gateL, gateR);
  
  //fitfunc->SetParameters(gArea, 1, gMean, gSigma);
  //fitfunc->SetParameters(607, 1, 2734, 130/2.35);
  fitfunc->SetParameters(gConstant, 50, 2734, gSigma);
  if (conv_centroid > 0) fitfunc->FixParameter(2,conv_centroid);  
  if (conv_fwhm > 0)     fitfunc->FixParameter(3,conv_fwhm/2.35482);
  
  
  fitfunc->SetLineColor(2);
  fitfunc->SetLineWidth(2);
  hist->Fit(fitfunc,"+Q L","same",gateL, gateR);   //low statistics - likelihood
  
  Integral   		= fitfunc->GetParameter(0);
  Integral_error    = fitfunc->GetParError(0);
  Lifetime          = fitfunc->GetParameter(1);
  Lifetime_error	= fitfunc->GetParError(1);
  Centroid          = fitfunc->GetParameter(2);
  Centroid_error	= fitfunc->GetParError(2);
  Sigma             = fitfunc->GetParameter(3);
  Sigma_error	    = fitfunc->GetParError(3);
  
  Chi2pNDF   = fitfunc->GetChisquare() / fitfunc->GetNDF();

  halflife       = 0.6931471806*Lifetime;
  halflife_error = 0.6931471806*Lifetime_error;
 	
  fprintf(out, " \t %4d-%4d  T(1/2)(b.l.)=%8.3f(%.3f) Centr=%8.3f(%.3f) Sigma=%8.3f(%.3f) Chi2/NDF=%2.1f ",
                 gateL, gateR, halflife, halflife_error, Centroid, Centroid_error, Sigma, Sigma_error, Chi2pNDF);
    printf(    " \t %4d-%4d  T(1/2)(b.l.)=%8.3f(%.3f) Centr=%8.3f(%.3f) Sigma=%8.3f(%.3f) Chi2/NDF=%2.1f ",
                 gateL, gateR, halflife, halflife_error, Centroid, Centroid_error, Sigma, Sigma_error, Chi2pNDF);
	
  
  label -> SetTextColor(2);
  label -> DrawText(.4, .75, Form("Binned Likelyhood  T(1/2) = %8.3f(%.3f)", halflife, halflife_error));
	
  
  
  
    
  fitfunc->SetLineColor(3);
  fitfunc->SetLineWidth(2);
  hist->Fit(fitfunc,"+Q","same",gateL, gateR);       //high statistics - chisq fit
  
  
  Integral   		= fitfunc->GetParameter(0);
  Integral_error    = fitfunc->GetParError(0);
  Lifetime          = fitfunc->GetParameter(1);
  Lifetime_error	= fitfunc->GetParError(1);
  Centroid          = fitfunc->GetParameter(2);
  Centroid_error	= fitfunc->GetParError(2);
  Sigma             = fitfunc->GetParameter(3);
  Sigma_error	    = fitfunc->GetParError(3);
  
  Chi2pNDF   = fitfunc->GetChisquare() / fitfunc->GetNDF();

  halflife       = 0.6931471806*Lifetime;
  halflife_error = 0.6931471806*Lifetime_error;
 	
  fprintf(out, " \t %4d-%4d  T(1/2)=%8.3f(%.3f) Centr=%8.3f(%.3f) Sigma=%8.3f(%.3f) Chi2/NDF=%2.1f ",
                 gateL, gateR, halflife, halflife_error, Centroid, Centroid_error, Sigma, Sigma_error, Chi2pNDF);
    printf(    " \t %4d-%4d  T(1/2)=%8.3f(%.3f) Centr=%8.3f(%.3f) Sigma=%8.3f(%.3f) Chi2/NDF=%2.1f ",
                 gateL, gateR, halflife, halflife_error, Centroid, Centroid_error, Sigma, Sigma_error, Chi2pNDF);
		
  label -> SetTextColor(3);
  label -> DrawText(.4, .7,  Form("Chi-squared             T(1/2) = %8.3f(%.3f)", halflife, halflife_error));
		
		

	
	
	
  fclose(out);


	}	



void write_area(TH1F *hist) {
	
	FILE *out = fopen("results.txt", "at");
	fprintf(out, "File '%s' - %s",files[1], &date_and_time);
	 printf(     "File '%s' - %s\n",files[1], &date_and_time);
	float a, da,  mean, dmean;
	     
	for (int i=0; i<ngates; i++) {
		
		a = hist->Integral(gateL[i],gateR[i]);
		da = sqrt(a);
		hist->GetXaxis()->SetRange(gateL[i],gateR[i]);
		mean = hist->GetMean();
		dmean = hist->GetMeanError();
		fprintf(out, "[full]%4d - %4d \t Centr=%8.3f(%.3f) A=%9.1f(%.3f)\n", gateL[i], gateR[i], mean, dmean, a, da);
		 printf(     "[full]%4d - %4d \t Centr=%8.3f(%.3f) A=%9.1f(%.3f)\n", gateL[i], gateR[i], mean, dmean, a, da);
    }
    fclose(out);
    
}

void write_area_bkg(TH1F *hist, TH1 *hist_bkg) {
	
	FILE *out = fopen("results.txt", "at");
    fprintf(out, "'%s' \t %s\n",files[1], &date_and_time);
     printf(     "'%s' \t %s\n",files[1], &date_and_time);
	
    	
	float a_tot, a_bkg, a, da,  mean, dmean;

	for (int i=0; i<ngates; i++) {
		a_tot  = hist    ->Integral(gateL[i],gateR[i]);
		hist->GetXaxis()->SetRange(gateL[i],gateR[i]);
		mean = hist->GetMean();
		dmean = hist->GetMeanError();
	    a_bkg  = hist_bkg->Integral(gateL[i],gateR[i]);
       
        a  = a_tot - a_bkg;
        da = sqrt(a_tot + a_bkg);
	    
		fprintf(out, "[-bkg]%4d \t %4d \t Centr=%8.3f(%.3f) A=%9.1f(%.3f)\n", gateL[i], gateR[i], mean, dmean, a, da);
		 printf(     "[-bkg]%4d \t %4d \t Centr=%8.3f(%.3f) A=%9.1f(%.3f)\n", gateL[i], gateR[i], mean, dmean, a, da);

		
   }

     fclose(out);
    
}

void peaksearch(TSpectrum *s, TH1F *hist) {
	
	 //to find the peak candidates
    int sigma = 1;
    float threshold = 0.03;
    int nfound = s->Search(hist,sigma,"",threshold);
    printf("Found %d candidate peaks \n",nfound);
  
}






int main( int argc, char **argv){
  
  
  printf("\n\t\t----------------------------------");
  printf("\n\t\t(simple) Spectrum analysis with ROOT");
  printf("\n\t\t           v15.12.2016  ");
  printf("\n\t\thttps://github.com/rlica/spec2root");
  printf("\n\t\t----------------------------------");
  printf("\n\n");

  timer();
  
    
  //Opening argument files                          
  open_files(argc, argv);

  //Reading gates if file was provided
  ngates = read_gates();

  //User's input                         
  interactive();
                              
  //Reading spectrum binary file
  data = (DATATYPE *)calloc(nchan, sizeof(DATATYPE));
  fread(data, sizeof(DATATYPE),nchan,file1);
  fclose(file1);
                            
  //Filling histogram
  TH1F *hist  =  new TH1F("hist" ,Form("%s", argv[1]), nchan,0,nchan);
  for (int i = 0; i < nchan; i++) hist->SetBinContent(i+1,data[i]); //i+1 to have same values as xtrackn
 

          
  //ROOT Graphical mode
  TApplication* rootapp = new TApplication("example", &argc, argv); //argc and argv will be reset

  
  //Use TSpectrum 
  TSpectrum *s = new TSpectrum(100);
   
            
  //Setting up the Canvas    
  TCanvas *Can1;
  Can1 = new TCanvas("Can1","Can1",800,800); 
  gStyle->SetOptStat(0);
  
  //Drawing initial histogram
  hist->Draw();

  //Gaussian Fit of time spectra
  if (time_spec == 1 && gauss_fit == 1) {
	  fit_gauss(hist, gateL[0], gateR[0]);
	  hist->GetXaxis()->SetRangeUser(gateL[0], gateR[0]);
	  }
	  
  //Expo Fit of time spectra
  if (time_spec == 1 && expo_fit == 1) {
	  fit_expo_pol0(hist, gateL[0], gateR[0]);
	  hist->GetXaxis()->SetRangeUser(2*gateL[0]-gateR[0], 2*gateR[0]-gateL[0]);
	  }
	  
  //Conv Fit of time spectra
  if (time_spec == 1 && conv_fit == 1) {
	  fit_conv(hist, gateL[0], gateR[0]);
	  hist->GetXaxis()->SetRangeUser(2*gateL[0]-gateR[0], 2*gateR[0]-gateL[0]);
	  }
	  	  
  //Automatic peaksearch
  //peaksearch(s,hist);


  //Writing areas of energy spectra (or gaussian fit)
  if      (time_spec == 0 && ngates > 0) write_area(hist);

  //Fitting the backround
  if (bkg_iterations != 0) {
  
    
    //Estimate background using TSpectrum::Background
    TH1 *hist_bkg = s->Background(hist,bkg_iterations,option_string);
    if (hist_bkg) Can1->Update();
    else cout << "Cannot perform background fitting" << endl;
  
      
    //Drawing spectrum with subtracted background
    Can1->cd(2);
    TH1F *hist2 = (TH1F*)hist->Clone();
    hist2->Add(hist_bkg,-1);
	hist2->SetLineColor(2);
    hist2->Draw("same");
    
    
    //Writing areas with subtracted background (or gaussian fit)
    if      (ngates > 0 && gauss_fit == 0) write_area_bkg(hist, hist_bkg);
	else if (ngates > 0 && gauss_fit == 1)
		for (int i=0; i<ngates; i++)
			fit_gauss(hist2, gateL[i], gateR[i]);

    
	}
        
    
  ///// Running the ROOT Graphical Application
  if (graphical == 1) {
  	cout << "To QUIT press [Ctrl+c] or ´File-Quit Root´ [Alt+f,q]" << endl;
  	rootapp->Run();
  }

    
  
  return(0);
}
