use IO::Socket;
use IO::Select;
$r = new IO::Select;
$sock=IO::Socket::INET->new(PeerAddr => $ARGV[0],PeerPort => $ARGV[1],Proto=>'tcp') or die "unable to connect: @ARGV";
print "Connected...\n";
$r->add($sock);
$r->add(STDIN);
while(1){
	($s)=IO::Select->select($r,undef,undef,0);
	foreach(@$s){
		if($_==$sock){
			$_ = <$sock>;
			print $_;
		}else{
			$_ = <STDIN>;
			print $sock $_;
		}
	}
}
