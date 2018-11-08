set WINSCP="C:\Program Files (x86)\WinSCP\WinSCP.com"
%WINSCP% /script=dosyncftp_maven.cmd
REM Publish artifact to MavenCentral
pushd ..\roborio\c++\navx_frc_cpp
call gradlew publish
popd
REM Publish artifact to MavenCentral
pushd ..\roborio\java\navx_frc
call gradlew publish
popd