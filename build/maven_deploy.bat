set WINSCP="C:\Program Files (x86)\WinSCP\WinSCP.com"
%WINSCP% /script=dosyncftp_maven.cmd
REM Publish artifact to MavenCentral
pushd ..\roborio\java\navx_frc
call gradlew publish
popd
REM Publish artifact to MavenCentral
pushd ..\roborio\c++\navx_frc_cpp
call gradlew publish
popd

ECHO To Release the MavenCentral Artifacts:
ECHO 
ECHO - Navigate to https://oss.sonatype.org/service/local/staging/deploy/maven2 and login
ECHO - At bottom of list, select the checkbox next to the staged content
ECHO - Click the Close button, which triggers analysis of the artifacts (takes about 1 minute)
ECHO - Then, click the Release button to migrate the staged build to the Release Repository
ECHO   It takes about 10 minutes to be released, and about 2 hours to show up in Maven Central Search.
