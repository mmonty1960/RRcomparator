# RRcomparator
Software for comparing the results got in the 3D-shape-measurement round-robin organized in the framework of the EU SFERA-III project

Source, MS executable and today available data (ENEA+FISE) are arranged in a Workspace directory to be ready to use.

The user can:
1) select one or more evaluators among ENEA, FISE, DLR, NREL, SANDIA, by ticking the respective checkBox
2) select one specimen among 3 inner (#60, #61, #62) and 3 outer (#93, #97, #99)
3) select one parameter among z(height), slopeX, slopeY, devZ, devSlopeX, devSlopeY
4) plot the 2D contour map for the selected evaluator, specimen, and parameter
5) launch the comparison between the results got from two evaluators, obtaining: mean, RMS, Peak-Valley, Min and Max values, as weel as the 2D contour map of the difference

The "Tab Parameter" of the Graphical User Interface (GUI) displays the values of the selected parameter at the 4 attaching points, named P1, P2, P3, P4, together the expected values for the ideal parabolic profile.
The "Tab XY ranges" of the GUI displays the X-range and Y-range of the loaded file.

The results are stored in the folder RRs3wp10. The results got for a given specimen are contained in a single file named as EVALUATOR_inner(outer)#N.dat composed by rows of x y z slopeX slopeY devZ devSlopeX devSlopeY values separated by a space. The preferred choice is X-step = Y-step = 10 mm; in the case of more dense sampling, data will be resampled in such a choice; in any case the software saves the resampled matrix in the file "resampledData.dat" (always in the folder RRs3wp10).

License:
The code source files are distributed as open source software under the GNU General Public
License as published by the Free Software Foundation version 3.

