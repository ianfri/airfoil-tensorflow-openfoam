#!/bin/bash
cd ${0%/*} || exit 1    # Run from this directory

# Source tutorial run functions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

#NUMFOILS=40

#for ((i = 0 ; i < $NUM_FOILS ; i++)); 
i=0
while (i < 40); do 
    # do all the openfoam stuff
    cp /mnt/c/Users/444in/Documents/xpsProgramming/r_nn_phy/$i.stl constant/triSurface/

    runApplication surfaceTransformPoints -yawPitchRoll "(0 0 90)" constant/triSurface/$i.stl constant/triSurface/tmpFoil.stl
    rm log.surfaceTransformPoints
    runApplication surfaceTransformPoints -translate "(0 0 3)" constant/triSurface/tmpFoil.stl constant/triSurface/tmpFoil2.stl
    rm constant/triSurface/$i.stl
    rm constant/triSurface/tmpFoil.stl
    mv constant/triSurface/tmpFoil2.stl constant/triSurface/$i.stl

    runApplication surfaceFeatures
    runApplication blockMesh
    runApplication decomposePar -copyZero
    runParallel snappyHexMesh -overwrite
    runParallel patchSummary
    runParallel potentialFoam
    runParallel simpleFoam
    runApplication reconstructParMesh -constant
    runApplication reconstructPar -latestTime

    # grab the last lift coefficient and store it in a csv
    # postProcessing/forceCoeffs1/0/forceCoeffs.dat
    lastLine = $( tail -n 1 postProcessing/forceCoeffs1/0/forceCoeffs.dat)
    echo $lastLine
    me = $( cut -f4 latestTime )
    echo $me

    # clear out the case
    ./Allclean

    i = i + 1
done






# Source tutorial run functions
. $WM_PROJECT_DIR/bin/tools/RunFunctions

# Copy foil surface from resources directory; ian: foil provided manually for now
#cp $FOAM_TUTORIALS/resources/geometry/foil.stl.gz constant/triSurface/
cp /mnt/c/Users/444in/Documents/xpsProgramming/r_nn_phy/foil.stl constant/triSurface/

runApplication surfaceTransformPoints -yawPitchRoll "(0 0 90)" constant/triSurface/foil.stl constant/triSurface/tmpFoil.stl
rm log.surfaceTransformPoints
runApplication surfaceTransformPoints -translate "(0 0 3)" constant/triSurface/tmpFoil.stl constant/triSurface/tmpFoil2.stl
rm constant/triSurface/foil.stl
rm constant/triSurface/tmpFoil.stl
mv constant/triSurface/tmpFoil2.stl constant/triSurface/foil.stl

runApplication surfaceFeatures

runApplication blockMesh

runApplication decomposePar -copyZero
runParallel snappyHexMesh -overwrite

runParallel patchSummary
runParallel potentialFoam
#runParallel $(getApplication)
runParallel simpleFoam

runApplication reconstructParMesh -constant
runApplication reconstructPar -latestTime

runApplication foamToVTK

#------------------------------------------------------------------------------
