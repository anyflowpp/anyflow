set f=%1%
set f=%f:/=\%
set t=%2%
set t=%t:/=\%
pushd %~dp0
    copy "%f%" "%t%"
    set r=%errorlevel%
    if not %r% == 0 (
        popd
        exit %r%
    )
popd