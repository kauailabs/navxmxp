- As of this writing, the navx_frc navx-cpp and navx-java artifacts are published to both 
  the Kauailabs maven repo as well as Maven Central.

- Maven account credentials are stored in %HOMEDRIVE%%HOMEPATH%\.gradle\gradle.properties

- During the build, the artifacts are published to the local repo at %HOMEDRIVE%%HOMEPATH%\.m2\repository\com.kauailabs\...

- maven_deploy.bat copies the local artifacts to the KauaiLabs Maven Repo
- maven_deploy.bat also publishes the artifacts to the MavenCentral staging repo.

NOTE:  After publishing to Maven Central, a manual process is required to release them:

- Login to issues.sonatype.org if access to account information or help is ever needed
- Navigate to https://oss.sonatype.org/service/local/staging/deploy/maven2 and login
- If upload succeeded, the library is at the bottom of the list. Here you can check the content of the upload.
- When it’s done, click the Close button in the bar above the list, this will trigger a set of analysis on your delivery. This can take some minutes to complete (if the interface doesn’t refresh, force the refresh in your browser) and you’ll get the result:

The tests failed: You can check which test failed, use the Drop button to delete your upload, fix the problem(s) and republish your library.

The tests succeeds: Congratulations! You can now click the Release button. If it’s your first upload, you have to mention it in commentary of the Jira ticket you opened in the first step. You can then find your artifact in the Maven Central Repository within 10 minutes and on Maven Central Search within 2 hours.