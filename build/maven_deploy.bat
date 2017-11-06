call mvn deploy:deploy-file -DpomFile=poms/navx/pom.xml -Dfile=..\java\navx\jar\navx.jar -DrepositoryId=kauailabs-maven-ftp -Durl=ftp://ftp.kauailabs.com/kauailabs.com/maven2

call mvn deploy:deploy-file -DpomFile=poms/navx_frc/pom.xml -Dfile=..\roborio\java\navx_frc\jar\navx_frc.jar -DrepositoryId=kauailabs-maven-ftp -Durl=ftp://ftp.kauailabs.com/kauailabs.com/maven2
