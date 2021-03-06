eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
     & eval 'exec perl -S $0 $argv:q'
     if 0;

# run_test.pl,v 1.3 2001/05/18 02:08:32 schmidt Exp
# -*- perl -*-

use English;

use lib '../../../bin';
use PerlACE::Run_Test;

if ($OSNAME eq "MSWin32") {
   $ENV{'PATH'} .= ';'.$PerlACE::Process::ExeSubDir;
} else {
   $ENV{'PATH'} .= ':'.$PerlACE::Process::ExeSubDir;
}

$T = new PerlACE::Process ("driver");

$test = $T->SpawnWaitKill (60);

if ($test != 0) {
    print STDERR "ERROR: test returned $test\n";
    exit 1;
}

exit 0;
