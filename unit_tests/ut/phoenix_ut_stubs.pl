#!/usr/bin/env perl
# /phoenix/unit_tests/ut/phoenix_ut_stabs.pl
#
# This file is a part of Phoenix operating system.
# Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

use File::Basename;
use Getopt::Long;

$NM = "nm";
$CPPFILT = "c++filt";
$result_name = "auto_stubs.cpp";
@SRCS = ( );
@TEST_SRCS = ( );

# All globally defined symbols
%defined_syms = ( );
# All undefined symbols
%wanted_syms = ( );

sub ParseFile {
    my ($filename, $isTest) = @_;
    
    open(NM, "$NM $filename |") or die("Failed to open file '$filename'");
    
    while (my $line = <NM>) {
        if ($line =~ /^\s*[0-9a-fA-F]*\s+([TDU])\s+(.*)$/) {
            if ($1 eq "T" or $1 eq "D") {
                $defined_syms{$2} = $filename;
            } elsif ($1 eq "U" and $isTest) {
                if ($2 !~ /__cxa|__cxx/) {
                    $wanted_syms{$2} = $filename;
                }
            }
        }
    }
    close(NM);
}

sub ResolveSymbols {
    for my $name (keys %wanted_syms) {
        if (defined $defined_syms{$name}) {
            delete $wanted_syms{$name};
        }
    }
}

sub DemangleName {
    my ($name) = @_;
    open(CPPFILT, "$CPPFILT $name |") or die ("Cannot open c++filt");
    my $readable_name = <CPPFILT>;
    close(CPPFILT);
    chomp($readable_name);
    return $readable_name;
}

sub CreateOutput {
    open(RESULT, "> $result_name") or die("Cannot open output file '$result_name'");
    
    my $header = <<EOF;
/* This file is automatically generated by Phoenix unit testing framework.
 * Do not edit it manually!
 */

#include <phoenix_ut.h>

#define SYM_STUB(idx, name, readable_name, wanted_by) \\
    namespace ut { \\
    void ut_auto_stub_ ## idx() asm(name); \\
    void ut_auto_stub_ ## idx() { \\
        UT_FAIL("Autogenerated stub for '" readable_name "' called\\n" \\
                "(referenced from '" wanted_by "')"); \\
    } \\
    } /* namespace ut */

EOF

    print(RESULT $header);
    
    my $symIdx = 0;
    for my $name (keys %wanted_syms) {
        my $readable_name = DemangleName($name);
        
        print(RESULT "SYM_STUB($symIdx,\n\t\"$name\",\n\t \"$readable_name\",\n\t\"$wanted_syms{$name}\")\n");
        $symIdx++;
    }

    close(RESULT);
    print("Stubs for $symIdx symbols generated\n");
}

GetOptions(
    "nm=s" => \$NM,
    "cppfilt=s" => \$CPPFILT,
    "result=s" => \$result_name,
    "src=s" => \@SRCS,
    "test_src=s" => \@TEST_SRCS
);

for my $file (@TEST_SRCS) {
    ParseFile($file, 1);
}

for my $file (@SRCS) {
    ParseFile($file, 0);
}

ResolveSymbols();

CreateOutput();
