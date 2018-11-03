set /p maven_version=< version.txt
pushd poms\navx
call mvn build-helper:parse-version versions:set -DnewVersion=%maven_version%
popd
pushd poms\navx_frc
call mvn build-helper:parse-version versions:set -DnewVersion=%maven_version%
popd
call mvn deploy:deploy-file -DpomFile=poms/navx/pom.xml -Dfile=..\java\navx\jar\navx.jar -DrepositoryId=kauailabs-maven-ftp -Durl=ftp://ftp.kauailabs.com/kauailabs.com/maven2 -Dversion=%maven_version%
call mvn deploy:deploy-file -DpomFile=poms/navx_frc/pom.xml -Dfile=..\roborio\java\navx_frc\jar\navx_frc.jar -DrepositoryId=kauailabs-maven-ftp -Durl=ftp://ftp.kauailabs.com/kauailabs.com/maven2 -Dversion=%maven_version%

