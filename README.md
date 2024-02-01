# RRcomparator
Software for comparing the results got in the 3D-shape-measurement round-robin (RR) hold in the framework of the EU SFERA-III project

Source, MS executable and data are arranged in a Workspace directory ready for use.

The user can:
1) select one or more evaluators among ENEA, FISE, DLR, NREL, SANDIA, by ticking the respective checkBox
2) select one specimen among 3 inner (#60, #61, #62) and 3 outer (#93, #97, #99)
3) select one parameter among z(height), slopeX, slopeY, devZ, devSlopeX, devSlopeY
4) plot the 2D contour map for the selected evaluator, specimen, and parameter
5) launch the comparison between the results got from two evaluators, or one evaluator and “ideal” or “mean” (among the loaded experimental 3D shape) profile, obtaining: mean, RMS, Peak-Valley, Min and Max values, as well as the 2D contour map of the difference

The Graphical User Interface (GUI) contains 3 Tabs:

- "Tab Parameter" displays the values of the selected parameter at the 4 attaching points, named P1, P2, P3, P4, together the expected values for the ideal parabolic profile (see the document RrplacingProcedure.pdf).

- “Tab XY ranges" displays X-range and Y-range of the loaded files.

- “Tab Statistics” displays Mean, RMS, Peak-Valley, Minimum and Maximum of the selected parameter for each evaluator

The results of the RR are stored in the folder “RRs3wp10”. The results got for a given specimen are contained in a single file named as

	EVALUATOR_inner(outer)#N.dat

composed by rows of x y z slopeX slopeY devZ devSlopeX devSlopeY values separated by a space. The slopes can be expressed as tan(theta) or theta in radiant. Parameters and units are specified in the first raw of the file. 

The preferred choice for the data gridding is X-step = Y-step = 10 mm; in the case of a more dense sampling, data will be resampled with the 10 mm step; in any case the software saves the resampled matrix used in the internal computing in the file "resampledData.dat", stored in the folder “RRs3wp10”.


INSTALLATION:
Download all the content from https://github.com/mmonty1960/RRcomparator by pushing the green button. Then unzip to a position at your choice on the PC.

Linux (preferred method):

- Download Manjaro Linux from https://manjaro.org with your preferred flavour and processor

- Install Manjaro Linux

- Refresh all packages

- Install QtCreator and opencv packages by means of the package manager

- Launch QtCreator and open the file “Rrcomparator.pro” contained in Workspace/qtSource/Rrcomparator/

- Complete the project configuration as asked (align all the paths to Workspace/qtSource/Rrcomparator/)

- Compile and enjoy

MS Windows:
Simply double-click on rrcmp_WinExecLauncher.bat file.

License:
The code source files are distributed as open source software under the GNU General Public
License as published by the Free Software Foundation version 3.


