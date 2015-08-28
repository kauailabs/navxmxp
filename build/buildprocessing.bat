set PROCESSING=C:\processing-3.0a5\processing-java
pushd ..
mkdir processing_output
%PROCESSING% --sketch=%CD%\processing\navXMXPUI --output=%CD%\processing_output --export --force
popd