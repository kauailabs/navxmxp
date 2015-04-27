REM Build binaries

pushd .\crio\java\nav6
call ant clean jar
popd

pushd .\roborio\java\navx-mxp
call ant clean jar
popd