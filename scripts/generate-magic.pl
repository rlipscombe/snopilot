#!/usr/bin/perl -w

$length = $ARGV[0] || 8;
$length >= 4 or die "$0: Magic must be at least 4 octets\n";
open(STDIN, "/dev/urandom") or die "$0: Can't open /dev/urandom: $!\n";

sub not_utf8($) {
    ($_[0]."\x80\x80\x80\x80\x80") !~ /\A[\x80-\xbf]{0,5}(
        [\x00-\x7f]|
        [\xc0-\xdf][\x80-\xbf]|
        [\xe0-\xef][\x80-\xbf]{2}|
        [\xf0-\xf7][\x80-\xbf]{3}|
        [\xf8-\xfb][\x80-\xbf]{4}|
        [\xfc-\xfd][\x80-\xbf]{5}
    )*\x80{0,5}\z/sx;
}

while(1) {
    sysread(STDIN, $magic, $length) or die "$0: /dev/urandom: $!\n";
    length($magic) == $length or die "$0: Short read\n";
    # no repeated octets
    $magic =~ /(.)\1/s and next;
    # at least 50% high-half
    $_ = $magic; $high = 0; s/[\x80-\xff]/$high++, "h"/seg;
    next unless $high*2 >= $length;
    # at least 75% not ASCII printable
    $_ = $magic; $asc = 0; s/[\x20-\x7e]/$asc++, "a"/seg;
    next if $asc*4 > $length;
    # at least one ASCII printable
    $magic =~ /[\x20-\x7e]/s or next;
    # at least one high-half ISO-8859 printable
    $magic =~ /[\xa0-\xff]/s or next;
    # at least one ISO-8859 control character
    $magic =~ /[\x00-\x08\x0b\x0e-\x1f\x7f-\x9f]/s or next;
    # not a substring of UTF-8
    not_utf8($magic) or next;
    not_utf8(reverse($magic)) or next;
    last;
}
$magic =~ s/(.)/sprintf("0x%02x ", ord($1))/seg;
$magic =~ s/ $/\n/;
print $magic;
