set INNOSETUP_COMPILER="C:\Program Files (x86)\Inno Setup 5\ISCC"
pushd ..\setup
%INNOSETUP_COMPILER% /ssigntool="signtool sign /a $f" vmx-pi-setup.iss
popd