my $app = sub {
    my $env = shift;
    return [200, ['Content-Type' => 'text/plain'], ["Test message from Plack.\n"]];
}
