#!/usr/local/bin/perl

# Written by Etienne Laurin

# A golfed command line plotter
# try ./plot.pl 'x**2' or 'cos(x/5)+1'

($e=pop)=~s/x/\$_/g or die;
for(0..46){$_[$_]=eval$e;}
($i,$a)=+(sort{$a<=>$b}@_)[0,-1];
@v=map{int(($_+$i)*20/($a-$i))}@_;
@_=();for(@v){$_=0 if $_<0;push @{$_[$_]},$l++;}
@_=reverse@_;
for(@_){$z=0;for(@$_){print' 'x($_-$z).'X';$z=$_+1;}print"\n";}
