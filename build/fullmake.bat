call cleanall
call makedocs
call dosyncftp_docs
call buildall
call makedist
call makedist_navX-Micro
call dosyncftp
rem call dosyncftp_navX-Micro
call maven_deploy
