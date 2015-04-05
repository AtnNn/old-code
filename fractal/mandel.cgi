#!/usr/local/bin/perl
use GD;
$|=1;
unless($t=$ENV{'QUERY_STRING'}){
	print <<EOW;
content-type: text/html

<html><head><title>mandel generator</title></head><body>
<h1>mandel generator</h1><form method=get>
zoom: <input type=text name=zoom value=1><br>
coordinates: (<input type=text name=x value=0>,
				<input type=text name=y value=0>)<br>
<input type=submit><br><Br><br>
<pre>
notes:
the script runs slow on slow machines
|x| and |y| should be < 2
and the zoom should not be to big
if u want the source, just ask
</pre>
</form></body></html>
EOW
exit;
}
my %o;
$t=~s/([^=&]+)=([0-9.\-]+)/$o{$1}=$2/ge;
#open F,'>last1';
print STDERR "$o{x}:$o{y}:$o{zoom}\n";
#close F;
print "content-type: image/jpeg\n\n";
my ($h,$w)=(200,200);
my $m=new GD::Image($h,$w);
open F,'colormap' or die 'could not load colour map';
my $max=0;
my @c;
while(<F>){
	$max++;
	die 'invalid colour map'
	if ($t=$m->colorAllocate(split/[^0-9]+/))==-1;
	push @c,$t;
}
close F;
sub P {
	my($x,$y)=@_;
	my $i,$a=0,$b=0;
	for($i=0;$i<$max;$i++){
		($a,$b)=($a*$a-$b*$b+$x,2*$a*$b+$y);
		last if $a*$a+$b*$b>4
	}
	return $i;
}
for($x=0;$x<$w;$x++){
	$px=(($x/$w-0.5)*4)/$o{zoom}+$o{x};
	for($y=0;$y<$h;$y++){
		$py=(($y/$h-0.5)*4)/$o{zoom}+$o{y};
		$m->setPixel($x,$y,P($px,$py));
	}
}
print $m->jpeg;
