call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
pushd ..
pushd c#
msbuild /p:Configuration=Release;Platform=x86 navXTools.sln
popd
popd
