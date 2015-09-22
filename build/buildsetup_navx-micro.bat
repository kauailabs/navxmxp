set INNOSETUP_COMPILER="C:\Program Files (x86)\Inno Setup 5\Compil32"
pushd ..\setup
%INNOSETUP_COMPILER% /cc navx-micro-setup.iss
popd