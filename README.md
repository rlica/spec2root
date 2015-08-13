# spec2root 
Interactive spectrum converter from binary format (GASPware) to root

## Author
Razvan Lica, CERN - IFIN-HH, razvan.lica@cern.ch

=========

## Installation and running
 1. Download via github.com or using the command line  
 `git clone https://github.com/rlica/spec2root`
 2. Compile with `make clean && make`.
 3. Add in `$HOME/.bashrc` or `$HOME/.profile`       
 `PATH=$PATH:/your_path_here/spec2root/`
 4. Run: `spec2root input_file gates_file[optional]`
 5. Enjoy the interactive session!


## Options

### Energy spectra
 * Automatic background subtraction.
 * Can output, in a batch mode, the areas based on the limits provided in the 'gates_file'
 * Areas can be extracted via integration, or gaussian fit.

### Time spectra
 * Can fit gaussian or exponential 


