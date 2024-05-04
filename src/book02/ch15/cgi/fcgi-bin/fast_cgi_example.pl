#!/usr/bin/perl
use CGI::Fast;

while (my $q = CGI::Fast->new)
{
    print("Content-Type: text/plain\n\n");
    foreach $var (sort(keys(%ENV)))
    {
        $val = $ENV{$var};
        $val =~ s|\n|\\n|g;
        $val =~ s|"|\\"|g;
        print "${var}=\"${val}\"\n";
    }
}
