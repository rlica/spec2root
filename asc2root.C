//  Read data from a two-column ascii histogram and create a root histogram.

//  Usage: root -l 'asc2root.C("File_name")'	
//  Author: R. Lica, O. Tengblad



void asc2root(char *filename) {

   ifstream in;
   in.open(filename);

   Float_t x,y;
   Int_t nlines = 0;
   TFile *f = new TFile(Form("%s.root", filename),"RECREATE");
   TH1F *h1 = new TH1F("h1","x distribution",30000,0,30000);
   
   while (1) {
      in >> x >> y ;
      if (!in.good()) break;
      //if (nlines < 5) printf("x=%8f, y=%8f\n",x,y);
      h1->SetBinContent(x,y);
      nlines++;
   }
   printf(" found %d points\n",nlines);

   in.close();

   f->Write();
}
